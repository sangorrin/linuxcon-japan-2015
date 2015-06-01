p_i_d=$(pgrep monitored-app)
sudo rmmod rt-monitor
sudo insmod rt-monitor.ko pid=$p_i_d
./loop_dmesg_last_line.sh
