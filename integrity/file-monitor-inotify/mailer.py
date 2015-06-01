import smtplib
from email.MIMEText import MIMEText

def send_sfm_report(filename):
	FROM = 'xxx.xxx@toshiba.co.jp'
	TO = 'xxx@toshiba.co.jp'

	content =  "This is a message from SFM"
	msg = MIMEText(content, 'plain')
	msg['Subject']= "TEST: the file " + filename + " was accessed!"
	msg['From']   = FROM
	msg['To']     = TO

	server = smtplib.SMTP("localhost")
	server.sendmail(FROM, [TO], msg.as_string())
	server.quit()

if __name__ == "__main__":
	print "sending mail"
	send_sfm_report("passwd")
