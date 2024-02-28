IMAGE = kolibri.img
DRIVE = ./newprogs

start:
	qemu-system-i386 -fda $(IMAGE) -boot a -m 512 -drive file=fat:rw:$(DRIVE)
