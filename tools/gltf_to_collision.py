#!/usr/bin/env python3

import struct
import sys
import json
import argparse
from pathlib import Path

try:
    from pygltflib import GLTF2
    import numpy as np
except ImportError:
    print("Error: Required packages not found. Install with:")
    print("  pip install pygltflib numpy")
    sys.exit(1)


COL_FLAG_WALKABLE = 0x0001
COL_FLAG_WALL = 0x0002
COL_FLAG_CEILING = 0x0004
COL_FLAG_TRIGGER = 0x0008

COL_FLAG_WATER = 0x0010
COL_FLAG_SNOW = 0x0020
COL_FLAG_WOOD = 0x0040
COL_FLAG_CEMENT = 0x0080

SURFACE_TYPE_MAP = {
    'water': COL_FLAG_WATER,
    'snow': COL_FLAG_SNOW,
    'wood': COL_FLAG_WOOD,
    'cement': COL_FLAG_CEMENT,
}

POSITION_SCALE = 16.0

NORMAL_SCALE = 127.0


def blender_to_n64(vec: np.ndarray) -> np.ndarray:
    return np.array([vec[0], vec[2], -vec[1]])


def load_gltf(filepath: str) -> GLTF2:
    return GLTF2().load(filepath)


def get_accessor_data(gltf: GLTF2, accessor_index: int) -> np.ndarray:
    accessor = gltf.accessors[accessor_index]
    buffer_view = gltf.bufferViews[accessor.bufferView]
    buffer = gltf.buffers[buffer_view.buffer]
    
    if buffer.uri is None:
        data = gltf.binary_blob()
    else:
        if buffer.uri.startswith('data:'):
            import base64
            data = base64.b64decode(buffer.uri.split(',')[1])
        else:
            bin_path = Path(gltf._path).parent / buffer.uri
            with open(bin_path, 'rb') as f:
                data = f.read()
    
    byte_offset = (buffer_view.byteOffset or 0) + (accessor.byteOffset or 0)
    
    component_types = {
        5120: np.int8,
        5121: np.uint8,
        5122: np.int16,
        5123: np.uint16,
        5125: np.uint32,
        5126: np.float32,
    }
    dtype = component_types[accessor.componentType]
    
    type_counts = {
        'SCALAR': 1,
        'VEC2': 2,
        'VEC3': 3,
        'VEC4': 4,
        'MAT2': 4,
        'MAT3': 9,
        'MAT4': 16,
    }
    count = type_counts[accessor.type]
    
    byte_length = accessor.count * count * np.dtype(dtype).itemsize
    arr = np.frombuffer(data, dtype=dtype, count=accessor.count * count, offset=byte_offset)
    
    if count > 1:
        arr = arr.reshape((accessor.count, count))
    
    return arr


def get_node_transform(node) -> np.ndarray:
    mat = np.eye(4)
    
    if node.matrix is not None:
        mat = np.array(node.matrix).reshape((4, 4)).T
    else:
        if node.scale is not None:
            s = np.diag([node.scale[0], node.scale[1], node.scale[2], 1.0])
            mat = mat @ s
        
        if node.rotation is not None:
            qx, qy, qz, qw = node.rotation
            r = np.array([
                [1-2*(qy*qy+qz*qz), 2*(qx*qy-qz*qw), 2*(qx*qz+qy*qw), 0],
                [2*(qx*qy+qz*qw), 1-2*(qx*qx+qz*qz), 2*(qy*qz-qx*qw), 0],
                [2*(qx*qz-qy*qw), 2*(qy*qz+qx*qw), 1-2*(qx*qx+qy*qy), 0],
                [0, 0, 0, 1]
            ])
            mat = r @ mat
        
        if node.translation is not None:
            t = np.eye(4)
            t[0, 3] = node.translation[0]
            t[1, 3] = node.translation[1]
            t[2, 3] = node.translation[2]
            mat = t @ mat
    
    return mat


def get_world_transform(gltf: GLTF2, node_index: int, parent_transform: np.ndarray = None) -> np.ndarray:
    if parent_transform is None:
        parent_transform = np.eye(4)
    
    node = gltf.nodes[node_index]
    local_transform = get_node_transform(node)
    return parent_transform @ local_transform


def transform_point(point: np.ndarray, matrix: np.ndarray) -> np.ndarray:
    p = np.array([point[0], point[1], point[2], 1.0])
    result = matrix @ p
    return result[:3]


def transform_normal(normal: np.ndarray, matrix: np.ndarray) -> np.ndarray:
    upper3x3 = matrix[:3, :3]
    try:
        inv_trans = np.linalg.inv(upper3x3).T
    except np.linalg.LinAlgError:
        inv_trans = upper3x3
    
    result = inv_trans @ normal
    length = np.linalg.norm(result)
    if length > 0.0001:
        result = result / length
    return result


def extract_triangles(gltf: GLTF2, model_scale: float = 1.0, material_surface_map: dict = None) -> list:
    triangles = []
    
    mat_to_surface = {}
    
    if gltf.materials:
        print(f"  Materials in GLB ({len(gltf.materials)}):")
        for mat_idx, mat in enumerate(gltf.materials):
            mat_name = mat.name or f"Material_{mat_idx}"
            print(f"    [{mat_idx}] '{mat_name}'")
    else:
        print("  No materials found in GLB")
    
    if material_surface_map and gltf.materials:
        for mat_idx, mat in enumerate(gltf.materials):
            mat_name = mat.name or f"Material_{mat_idx}"
            if mat_name in material_surface_map:
                mat_to_surface[mat_idx] = material_surface_map[mat_name]
                print(f"  Material '{mat_name}' -> surface type 0x{material_surface_map[mat_name]:04x}")
    
    mesh_to_nodes = {}
    for node_idx, node in enumerate(gltf.nodes):
        if node.mesh is not None:
            if node.mesh not in mesh_to_nodes:
                mesh_to_nodes[node.mesh] = []
            mesh_to_nodes[node.mesh].append(node_idx)
    
    node_parents = {i: None for i in range(len(gltf.nodes))}
    for node_idx, node in enumerate(gltf.nodes):
        if node.children:
            for child_idx in node.children:
                node_parents[child_idx] = node_idx
    
    def compute_world_transform(node_idx):
        node = gltf.nodes[node_idx]
        local = get_node_transform(node)
        parent_idx = node_parents[node_idx]
        if parent_idx is not None:
            parent_world = compute_world_transform(parent_idx)
            return parent_world @ local
        return local
    
    for mesh_idx, mesh in enumerate(gltf.meshes):
        node_indices = mesh_to_nodes.get(mesh_idx, [])
        if not node_indices:
            node_indices = [None]
        
        for node_idx in node_indices:
            if node_idx is not None:
                world_transform = compute_world_transform(node_idx)
            else:
                world_transform = np.eye(4)
            
            scale_mat = np.diag([model_scale, model_scale, model_scale, 1.0])
            world_transform = scale_mat @ world_transform
            
            for primitive in mesh.primitives:
                if primitive.mode is not None and primitive.mode != 4:
                    continue
                
                if 'POSITION' not in primitive.attributes.__dict__ or primitive.attributes.POSITION is None:
                    continue
                
                prim_material_idx = primitive.material
                surface_flag = mat_to_surface.get(prim_material_idx, 0) if prim_material_idx is not None else 0
                
                positions = get_accessor_data(gltf, primitive.attributes.POSITION)
                
                if primitive.indices is not None:
                    indices = get_accessor_data(gltf, primitive.indices).flatten()
                else:
                    indices = np.arange(len(positions))
                
                if hasattr(primitive.attributes, 'NORMAL') and primitive.attributes.NORMAL is not None:
                    normals = get_accessor_data(gltf, primitive.attributes.NORMAL)
                else:
                    normals = None
                
                for i in range(0, len(indices), 3):
                    if i + 2 >= len(indices):
                        break
                    
                    i0, i1, i2 = indices[i], indices[i+1], indices[i+2]
                    
                    local_v0 = positions[i0]
                    local_v1 = positions[i1]
                    local_v2 = positions[i2]
                    
                    world_v0 = transform_point(local_v0, world_transform)
                    world_v1 = transform_point(local_v1, world_transform)
                    world_v2 = transform_point(local_v2, world_transform)
                    
                    v0 = blender_to_n64(world_v0)
                    v1 = blender_to_n64(world_v1)
                    v2 = blender_to_n64(world_v2)
                    
                    if normals is not None:
                        local_n = (normals[i0] + normals[i1] + normals[i2]) / 3.0
                        world_n = transform_normal(local_n, world_transform)
                        n = blender_to_n64(world_n)
                        length = np.linalg.norm(n)
                        if length > 0.0001:
                            n = n / length
                        else:
                            n = np.array([0, 1, 0])
                    else:
                        edge1 = v1 - v0
                        edge2 = v2 - v0
                        n = np.cross(edge1, edge2)
                        length = np.linalg.norm(n)
                        if length > 0.0001:
                            n = n / length
                        else:
                            n = np.array([0, 1, 0])
                    
                    flags = 0
                    if n[1] >= 0.7:
                        flags |= COL_FLAG_WALKABLE
                    elif n[1] < -0.7:
                        flags |= COL_FLAG_CEILING
                    else:
                        flags |= COL_FLAG_WALL
                    
                    flags |= surface_flag

                    triangles.append({
                        'v0': v0,
                        'v1': v1,
                        'v2': v2,
                        'normal': n,
                        'material': prim_material_idx if prim_material_idx is not None else 0,
                        'flags': flags,
                    })
    
    return triangles


def calculate_aabb(triangles: list) -> tuple:
    if not triangles:
        return (0, 0, 0), (0, 0, 0)
    
    min_pos = np.array([float('inf'), float('inf'), float('inf')])
    max_pos = np.array([float('-inf'), float('-inf'), float('-inf')])
    
    for tri in triangles:
        for v in [tri['v0'], tri['v1'], tri['v2']]:
            min_pos = np.minimum(min_pos, v)
            max_pos = np.maximum(max_pos, v)
    
    return tuple(min_pos), tuple(max_pos)


def write_collision_binary(triangles: list, output_path: str):
    
    aabb_min, aabb_max = calculate_aabb(triangles)
    
    with open(output_path, 'wb') as f:
        f.write(b'COL1')
        
        f.write(struct.pack('>H', 1))
        
        f.write(struct.pack('>H', 0))
        
        f.write(struct.pack('>I', len(triangles)))
        
        for i in range(3):
            val = int(aabb_min[i] * POSITION_SCALE)
            val = max(-32768, min(32767, val))
            f.write(struct.pack('>h', val))
        
        for i in range(3):
            val = int(aabb_max[i] * POSITION_SCALE)
            val = max(-32768, min(32767, val))
            f.write(struct.pack('>h', val))
        
        f.write(struct.pack('>I', 0))
        
        for tri in triangles:
            for i in range(3):
                val = int(tri['v0'][i] * POSITION_SCALE)
                val = max(-32768, min(32767, val))
                f.write(struct.pack('>h', val))
            
            for i in range(3):
                val = int(tri['v1'][i] * POSITION_SCALE)
                val = max(-32768, min(32767, val))
                f.write(struct.pack('>h', val))
            
            for i in range(3):
                val = int(tri['v2'][i] * POSITION_SCALE)
                val = max(-32768, min(32767, val))
                f.write(struct.pack('>h', val))
            
            for i in range(3):
                val = int(tri['normal'][i] * NORMAL_SCALE)
                val = max(-128, min(127, val))
                f.write(struct.pack('>b', val))
            
            f.write(struct.pack('>B', tri['material']))
            
            f.write(struct.pack('>H', tri['flags']))
            
            f.write(b'\x00' * 8)
    
    return len(triangles)


def load_surface_config(glb_path: str, verbose: bool = False) -> dict:
    json_path = Path(glb_path).with_suffix('.json')
    material_surface_map = {}
    
    if not json_path.exists():
        if verbose:
            print(f"  No surface config found: {json_path}")
        return material_surface_map
    
    if verbose:
        print(f"  Loading surface config: {json_path}")
    
    try:
        with open(json_path, 'r') as f:
            config = json.load(f)
        
        surfaces = config.get('surfaces', {})
        for surface_type, material_names in surfaces.items():
            surface_type = surface_type.lower()
            if surface_type not in SURFACE_TYPE_MAP:
                print(f"  Warning: Unknown surface type '{surface_type}', skipping. Valid: {', '.join(SURFACE_TYPE_MAP.keys())}")
                continue
            
            surface_flag = SURFACE_TYPE_MAP[surface_type]
            
            if isinstance(material_names, str):
                material_names = [material_names]
            
            for mat_name in material_names:
                material_surface_map[mat_name] = surface_flag
                if verbose:
                    print(f"    {mat_name} -> {surface_type}")
        
        if verbose and material_surface_map:
            print(f"  Loaded {len(material_surface_map)} material mappings")
    
    except json.JSONDecodeError as e:
        print(f"  Warning: Failed to parse {json_path}: {e}")
    except Exception as e:
        print(f"  Warning: Error loading {json_path}: {e}")
    
    return material_surface_map


def main():
    parser = argparse.ArgumentParser(
        description='Convert glTF mesh to N64 collision binary format',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python gltf_to_collision.py level.glb level.bcol
  
  python gltf_to_collision.py level.glb level.bcol --list-materials

Surface types: snow, wood, cement, water
"""
    )
    parser.add_argument('input', help='Input glTF/glb file')
    parser.add_argument('output', help='Output collision file (.bcol)')
    parser.add_argument('--scale', type=float, default=1.0,
                        help='Scale factor for the model (default: 1.0)')
    parser.add_argument('--list-materials', action='store_true',
                        help='List all material names in the file and exit')
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='Print detailed information')
    
    args = parser.parse_args()
    
    input_path = Path(args.input)
    if not input_path.exists():
        print(f"Error: Input file not found: {args.input}")
        sys.exit(1)
    
    if args.verbose:
        print(f"Loading: {args.input}")
    
    gltf = load_gltf(args.input)
    
    if args.verbose:
        print(f"  Meshes: {len(gltf.meshes)}")
    
    if args.list_materials:
        print("\nMaterials in file:")
        if gltf.materials:
            for idx, mat in enumerate(gltf.materials):
                mat_name = mat.name or f"Material_{idx}"
                print(f"  [{idx}] {mat_name}")
        else:
            print("  (no materials found)")
        sys.exit(0)
    
    material_surface_map = load_surface_config(args.input, args.verbose)
    
    triangles = extract_triangles(gltf, args.scale, material_surface_map)
    
    if args.verbose:
        print(f"  Triangles: {len(triangles)}")
    
    if not triangles:
        print("Warning: No triangles found in the model")
    
    aabb_min, aabb_max = calculate_aabb(triangles)
    if args.verbose:
        print(f"  AABB min: ({aabb_min[0]:.2f}, {aabb_min[1]:.2f}, {aabb_min[2]:.2f})")
        print(f"  AABB max: ({aabb_max[0]:.2f}, {aabb_max[1]:.2f}, {aabb_max[2]:.2f})")
    
    if args.verbose:
        walkable = sum(1 for t in triangles if t['flags'] & COL_FLAG_WALKABLE)
        walls = sum(1 for t in triangles if t['flags'] & COL_FLAG_WALL)
        ceilings = sum(1 for t in triangles if t['flags'] & COL_FLAG_CEILING)
        print(f"  Walkable faces: {walkable}")
        print(f"  Wall faces: {walls}")
        print(f"  Ceiling faces: {ceilings}")
        
        snow = sum(1 for t in triangles if t['flags'] & COL_FLAG_SNOW)
        wood = sum(1 for t in triangles if t['flags'] & COL_FLAG_WOOD)
        cement = sum(1 for t in triangles if t['flags'] & COL_FLAG_CEMENT)
        water = sum(1 for t in triangles if t['flags'] & COL_FLAG_WATER)
        if snow or wood or cement or water:
            print(f"  Surface types - Snow: {snow}, Wood: {wood}, Cement: {cement}, Water: {water}")
    
    count = write_collision_binary(triangles, args.output)
    
    file_size = 24 + (count * 32)
    
    print(f"Wrote {count} triangles to {args.output} ({file_size} bytes)")


if __name__ == '__main__':
    main()
