_pid=$(pgrep monitored-app)
sudo insmod ./change_page.ko pid=$_pid
