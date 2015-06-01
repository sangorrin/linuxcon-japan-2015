gcc monitored-app.c -static -o monitored-app
pid=$(pgrep monitored-app)
if [ $pid > 0 ] ;then
echo "The monitored-app exist!"
echo "pid=$pid"
rm 0
else
rm 0
sudo ./monitored-app
fi
