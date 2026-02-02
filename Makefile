BUILD_DIR=build
T3D_INST=$(shell realpath ../)

include $(N64_INST)/include/n64.mk
include $(T3D_INST)/t3d.mk

N64_CFLAGS += -std=gnu2x -Os
N64_C_AND_CXX_FLAGS += -Iinclude

T3D_GLCOL_TO_BCOL=tools/gltf_to_collision.py

vpath %.glb assets/mdl assets/mdl/player assets/mdl/map assets/mdl/npc assets/mdl/test assets/mdl/fish
vpath %.png assets/img assets/mdl assets/mdl/player assets/mdl/map assets/mdl/npc assets/mdl/test assets/mdl/fish

assets_png = $(shell find assets -name '*.png' 2>/dev/null)
assets_gltf = $(shell find assets/mdl -name '*.glb' 2>/dev/null)
assets_wav = $(wildcard assets/snd/*.wav)
assets_mp3 = $(wildcard assets/snd/*.mp3)
assets_glcol = $(wildcard assets/col/*.glb)
assets_xm = $(wildcard assets/mus/*.xm)
assets_ttf = $(wildcard assets/font/*.ttf)
assets_conv = $(addprefix filesystem/,$(notdir $(assets_png:%.png=%.sprite))) \
			  $(addprefix filesystem/,$(notdir $(assets_ttf:%.ttf=%.font64))) \
			  $(addprefix filesystem/,$(notdir $(assets_wav:%.wav=%.wav64))) \
			  $(addprefix filesystem/,$(notdir $(assets_mp3:%.mp3=%.wav64))) \
			  $(addprefix filesystem/,$(notdir $(assets_gltf:%.glb=%.t3dm))) \
			  $(addprefix filesystem/,$(notdir $(assets_xm:%.xm=%.xm64))) \
			  $(addprefix filesystem/,$(notdir $(assets_glcol:%.glb=%.bcol)))

src = src/core/main.cpp src/core/sound.cpp src/core/music.cpp src/core/camera.cpp src/core/actor.cpp src/core/viewport.cpp src/core/model.cpp src/core/skinned_model.cpp src/core/light.cpp src/core/player.cpp src/core/particle.cpp src/core/wipe.cpp src/core/collision.cpp src/core/textbox.cpp src/core/scene.cpp src/core/menu.cpp src/core/anim_controller.cpp src/core/player_state.cpp src/core/shop.cpp src/core/save_manager.cpp

all: bug.z64

filesystem/%.xm64: assets/mus/%.xm
	@mkdir -p $(dir $@)
	@echo "    [AUDIO] $@"
	@$(N64_AUDIOCONV)  --xm-compress 0 -o filesystem "$<"

filesystem/%.wav64: assets/snd/%.wav
	@mkdir -p $(dir $@)
	@echo "    [AUDIO] $@"
	@$(N64_AUDIOCONV) --wav-compress 1 -o filesystem $<

filesystem/%.wav64: assets/snd/%.mp3
	@mkdir -p $(dir $@)
	@echo "    [AUDIO] $@"
	@$(N64_AUDIOCONV) --wav-compress 1 -o filesystem $<

filesystem/%.sprite: %.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	$(N64_MKSPRITE) $(MKSPRITE_FLAGS) -o filesystem "$<"

filesystem/%.font64: assets/font/%.ttf
	@mkdir -p $(dir $@)
	@echo "    [FONT] $@"
	$(N64_MKFONT) $(MKFONT_FLAGS) -o filesystem "$<"

filesystem/%.t3dm: %.glb
	@mkdir -p $(dir $@) build/gltf_temp/$*/assets build/gltf_temp/$*/filesystem
	@echo "    [T3D-MODEL] $@"
	@cp "$<" build/gltf_temp/$*/assets/model.glb
	@if [ -d "$$(dirname "$<")" ]; then find "$$(dirname "$<")" -maxdepth 1 -name "*.png" -exec cp {} build/gltf_temp/$*/assets/ \; ; fi
	@cd build/gltf_temp/$* && $(T3D_GLTF_TO_3D) assets/model.glb filesystem/$*.t3dm
	@mv build/gltf_temp/$*/filesystem/$*.t3dm $@
	@find build/gltf_temp/$*/filesystem -name "$*.*.sdata" -exec mv {} filesystem/ \; 2>/dev/null || true
	$(N64_BINDIR)/mkasset  -o filesystem $@

filesystem/%.bcol: assets/col/%.glb	
	@mkdir -p $(dir $@)
	@echo "    [T3D-COLLISION] $@"
	@python3 $(T3D_GLCOL_TO_BCOL) -v "$<" $@

$(BUILD_DIR)/bug.dfs: $(assets_conv)
$(BUILD_DIR)/bug.elf: $(src:%.cpp=$(BUILD_DIR)/%.o)

bug.z64: $(BUILD_DIR)/bug.dfs
bug.z64: N64_ROM_TITLE="BUG GAME"
bug.z64: N64_ROM_SAVETYPE=eeprom4k

clean:
	rm -rf $(BUILD_DIR) *.z64
	rm -rf filesystem

build_lib:
	rm -rf $(BUILD_DIR) *.z64
	make -C $(T3D_INST)
	make all

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
