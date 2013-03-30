#! /bin/bash
# /etc/init.d/emailip

### BEGIN INIT INFO
# Provides:          emailip
# Required-Start:    $remote_fs $syslog
# Required-Stop:     $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: emails IP on boot
# Description:       sends an email to saites2001@gmail.com with the output of ifconfig
### END INIT INFO

case "$1" in
  start)
	/sbin/ifconfig | /usr/bin/mail -s "Raspberry Pi IP Address" saites2001@gmail.com
	;;
  stop)
	;;
  *)
	echo "Usage: /etc/init.d/emailip start"
	exit 1
	;;
esac

exit 0