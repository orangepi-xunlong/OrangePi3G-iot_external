obj-y += ft_gesture_lib.o
$(obj)/ft_gesture_lib.o: $(srctree)/$(obj)/touchpanel/ft_gesture_lib.o
	cp $(srctree)/$(obj)/touchpanel/ft_gesture_lib.o $(obj)/ft_gesture_lib.o
