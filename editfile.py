#!/usr/bin/env python
import io
import re

def editcommand( str ):
	#read the file
	with open('/var/www/html/.htaccess', 'r') as file:
		data = file.readlines()

	#find the indices of the old command to be replaced
	indices = re.finditer(':', data[1])
	indices_array = [index.start() for index in indices]

	#get the string value of the old command and replace it
	old_command = data[1][indices_array[0]+1:indices_array[1]]
	new_data = data[1].replace(old_command,str)

	#set the new command in the data string
	data[1] = new_data

	#write the new contents to the file
	with open('/var/www/html/.htaccess', 'w') as file:
		file.writelines(data)
	return True
