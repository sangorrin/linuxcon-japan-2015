#Detele all the .dump files
rm -f *dump

#Delete all the objdump* and od* file
rm -f objdump_* od_*

#Get the pid of the process 'monitored-app'
pid=$(pgrep "monitored-app")

#Starting to create .text section for '$ objdump' command
#Step1. Get all the data using $objdump -s -j 
objdump -s -j .text monitored-app > objdump_original

#Step2. Get the useful information in the objdump_original
cat objdump_original | awk '{print $2, $3, $4, $5}' | sed '$d' | tail -n +5 >objdump_text_section
rm objdump_original
#End

#Starting to create .text section for '$ od' command
#dump all memory and then use od to get all the dump file data 'into od_original' file
sudo ./dump-all-memory-of-pid.sh $pid
od -v -t x1 $pid* > od_original
cat od_original | awk '{for(i=2;i<=NF;i++) printf $i""FS;print ""}' | tr -s '\n' | sed 's/ //g' > od_temp
cat od_temp | sed 's/./& /8' | sed 's/./& /17' | sed 's/./& /26' > od_all_data
rm od_temp
rm od_original

#We need to get some useful information about the file 'objdump_text_section'
first_line_comment=$(head -1 objdump_text_section)
total_line_number=$(cat objdump_text_section | wc -l)

#We match the first_line_comment in the 'od_all_data' file
start_line_number=$(cat od_all_data | sed -n "/$first_line_comment/=")
#echo $start_line_number

#Get the text section of 'od_all_data'
cat od_all_data | tail -n +$start_line_number | head -n $total_line_number > od_text_section
rm od_all_data
rm -f *.dump
rm 0

echo "Hash value of files:"
sha1sum objdump_text_section
sha1sum od_text_section
