.PHONY: vis
vis: punches.in punchpress-visualization/target/punchpressvis/bin/punchpressvis
	punchpress-visualization/target/punchpressvis/bin/punchpressvis -c /dev/ttyACM0 -i punches.in -o punches.out

punchpress-visualization/pom.xml:
	git submodule update --init --recursive

punchpress-visualization/target/punchpressvis/bin/punchpressvis: punchpress-visualization/pom.xml punchpress-visualization/jlink
	bash -c "cd punchpress-visualization; mvn javafx:jlink"

punchpress-visualization/jlink:
	ln -s /usr/lib/jvm/java-18-openjdk/bin/jlink punchpress-visualization/jlink

punches.in:
	python gen-punches.py > punches.in

.PHONY: flash
flash: controller/Debug/makefile
	make -j8 -C controller/Debug/ all
	arm-none-eabi-objcopy -O binary controller/Debug/controller.elf controller/Debug/controller.bin
	st-flash write controller/Debug/controller.bin 0x8000000
	st-flash reset

.PHONY: flash-simulator
flash-simulator: punchpress-simulator.bin
	st-flash write punchpress-simulator.bin 0x8000000
	st-flash reset

.PHONY: flash-sample-controller
flash-sample-controller: punchpress-sample-controller.bin
	st-flash write punchpress-sample-controller.bin 0x8000000
	st-flash reset

punchpress-sample-controller.bin: punchpress-sample-controller.elf
	arm-none-eabi-objcopy -O binary punchpress-sample-controller.elf punchpress-sample-controller.bin

punchpress-simulator.bin: punchpress-simulator.elf
	arm-none-eabi-objcopy -O binary punchpress-simulator.elf punchpress-simulator.bin
	
state_machine.pdf: state_machine.dot
	dot state_machine.dot -Tpdf > state_machine.pdf
