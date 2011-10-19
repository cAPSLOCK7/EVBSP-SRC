#!/system/bin/sh

insmod /lib/modules/inter_dsp.ko
insmod /lib/modules/em_ave.ko
 
ln -s /dev/v4l/video0 /dev/video0
ln -s /dev/v4l/video1 /dev/video1

mkdir /dev/fb
ln -s /dev/graphics/fb0 /dev/fb/0

ls /lib/modules/pvrsrvkm.ko
ret=$?

case ${ret} in
  0)
    insmod /lib/modules/pvrsrvkm.ko
    chmod 666 /dev/pvrsrvkm
    /system/bin/pvrsrvinit
    ;;
esac

