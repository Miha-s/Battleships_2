
let view = {
	displayMessage: function(msg) {
		var messageArea = document.getElementById("message");
		messageArea.innerHTML = msg;
	},
	displayCell(what, where, position) {},
	clearField(whose) {}
};

view.displayCell = function(what, where, position) {
	if(what == "hit")
		what = "hit-ship";
	else
		what = "miss";
	let id = (where == "player" ? "" : "i");
	position += id;
	
	let cell = document.getElementById(position);
	cell.setAttribute("class", what);
}

let parameters = {
	player_field: [],
	oponent_field: [],
	ships_sizes: new Map([
	// first number is ship length, and second - amount of such ships
	[1,1],
		// [1, 4],
		// [2, 3],
		// [3, 2],
		// [4, 1],
	]),
	ships_count:  0,
	clear_field: function(field) {
		for(i = 0; i < 10; i++) {
			field[i] = [];
			for(k = 0; k < 10; k++)
				field[i][k] = 0;
		}
	},
	
};

parameters.ships_sizes.forEach((val) => parameters.ships_count += val);

let game = {
	getPosition: {
		last_length: 0,
		last_coords: [],
		count_ships_remain: parameters.ships_count,
		ships_remain: parameters.ships_sizes,
		parseShip() {},
		validateInput(coords) {},
		correctPosition(coords) {}, 
		setData(coords) {}, 
		setPicture() {},
		validateShot(coords) {},
		process_clicks: {
			last_clicks: [],
			click() {},
			setCallbacks() {},
			click_fire() {},
			clear_field() {},
			removeCallbacks() {},
		},
	},
	last_shot: "N",
	initGame() {},
	setShips(file, callback) {}, 
	shot() {},
	handleShot() {}, 
	shotcallback() {},
	setcallback() {},
};

game.getPosition.process_clicks.clear_field = function() {
	this.last_clicks.forEach((e) => { let el = document.getElementById(e);
									  el.removeAttribute("class"); });
	this.last_clicks = [];
}


game.getPosition.process_clicks.click = function() {
	if(this.hasAttribute("class"))
		return;
	let Input = document.getElementById("guessInput");

	let val = Input.value;
	if(game.getPosition.process_clicks.last_clicks.length != 0)
		val += " ";
	val += this.id;
	Input.value = val;
	this.setAttribute("class", "choose");
	game.getPosition.process_clicks.last_clicks.push(this.id);
}

game.getPosition.process_clicks.click_fire = function() {
	let Input = document.getElementById("guessInput");
	Input.value = this.id[0] + this.id[1];
	let but = document.getElementById("fireButton");
	but.onclick();
}

game.getPosition.process_clicks.setCallbacks = function() {
	for(let i = 0; i <  10; i++)
		for(let j = 0; j < 10; j++) {
			let id = String(i) + String(j);
			let el = document.getElementById(id);
			el.onclick = this.click;
			el = document.getElementById(id+"i");
			el.onclick = this.click_fire;
		}
}

game.getPosition.process_clicks.removeCallbacks = function() {
	for(let i = 0; i < 10; i++)
		for(let j = 0; j < 10; j++) {
			let id = String(i) + String(j);
			let el = document.getElementById(id);
			el.onclick = function() {};
		}
}	

game.getPosition.correctPosition = function(coords) {
	let first, second;
	if(coords[0][0] == coords[1][0]) { // horizontal ship
		first = 0;
		second = 1;
	} else if(coords[0][1] == coords[1][1]) { // vertical position
		first = 1;
		second = 0;
	} else {
		return false;
	}
	for(i = 0; i < this.last_length-1; i++) {
		if(coords[i][first] != coords[i+1][first])
			return false;
		if(Math.abs(coords[i][second] - coords[i+1][second]) > 1)
			return false;
	}
	return true;
}

game.getPosition.validateInput = function(coords) {
	if(!this.count_ships_remain) // all ships entered
		return false;

	if(!parameters.player_field.length) // the field wasn't created
		parameters.clear_field(parameters.player_field);

	let arr_coords = coords.split(" ");
	this.last_length = arr_coords.length;

	let tmp = this.ships_remain.get(this.last_length);
	if(tmp == undefined || tmp == 0) // ships of such length are already entered
		return false;
	let arr_int_coords = [];
	for(i = 0; i < arr_coords.length; i++) {
		let row = Number(arr_coords[i][0]);
		let col = Number(arr_coords[i][1]);
		if(Number.isNaN(row)|| Number.isNaN(col))
			return false;
		if(parameters.player_field[row][col])
			return false;
		let elem = [row, col];
		let repeat = false;
		arr_int_coords.forEach((val) => {
			if(val[0] == elem[0] && val[1] == elem[1])
				repeat = true;
		});
		if(repeat)
			return false; 
		arr_int_coords[i] = elem;
	}	

	this.last_coords = arr_int_coords;
	
	if(this.last_length == 1)
		return true;
	
	if(!this.correctPosition(arr_int_coords))
		return false;
	
	return true;
}
	
game.getPosition.setData = function(coords) {
	this.count_ships_remain--;
	let count = this.ships_remain.get(this.last_length);
	this.ships_remain.set(this.last_length, count-1);
	coords.forEach(val => parameters.player_field[val[0]][val[1]] = 1);
}

game.getPosition.setPicture = function() {
	this.last_coords.forEach((val) => {
		let id = String(val[0]) + String(val[1]);
		let cell = document.getElementById(id);
		cell.setAttribute("class", "ship");
	});
}


game.getPosition.parseShip = function() {
	game.getPosition.process_clicks.clear_field();
	let Input = document.getElementById("guessInput");
	let coords = Input.value;
	Input.value = "";
	if(!game.getPosition.validateInput(coords))
		return ;
	
	// set ship
	game.getPosition.setData(game.getPosition.last_coords);

	// set pictures
	game.getPosition.setPicture();

	if(game.getPosition.count_ships_remain == 0)
	game.initGame();
}

game.getPosition.validateShot = function(coords) {
	if(Number.isNaN(Number(coords[0])))
		return false;
	if(Number.isNaN(Number(coords[1])))
		return false;
	return true;
}

game.shot = function () {
	let Input = document.getElementById("guessInput");
	let coords = Input.value;
	Input.value = "";
	if(!game.getPosition.validateShot(coords))
		return;
	
	game.last_shot = coords[0] + coords[1];
	let file = "/game?" + game.last_shot;
	game.setShips(file, game.shotcallback);
}

game.shotcallback = function () {
	if(this.readyState < 4)
		return ;
	let mes = this.responseText;
	if(mes == "N")
		return ;
	if(mes == "W" || mes == "L") {
		processEnd(mes);
		return ;
	}
	if(mes == "+") {
		view.displayMessage("You hit him!");
		hit = "hit";
	} else {
		view.displayMessage("Oh, you miss...");
		hit = "miss";
	}
	view.displayCell(hit, "oponent", game.last_shot);
	game.setShips("/game?g", game.handleShot);	// waiting for other player to fire
}

game.processEnd = function(mes) {
	if(mes == "W")
		view.displayMessage("Yey! You Win");
	else
		view.displayMessage("Oh... you lose...");
	let but = document.getElementById("fireButton");
	but.onclick = null;
	
}

game.handleShot = function() {
	if(this.readyState < 4)
		return;
	let mes = this.responseText;
	if(mes == "N")
		return;
	if(mes == "W" || mes == "L") {
		game.processEnd(mes);
		return ;
	}
	let arr = mes.split("\n");
	mes = arr[0];
	let y = Number(mes[0]);
	let x = Number(mes[1]);
	let hit;
	if(parameters.player_field[y][x])
		hit = "hit";
	else
		hit = "miss";
	view.displayCell(hit, "player", mes);
	if(arr.length > 1)
		game.processEnd(arr[1]);
}
	

game.setShips = function(file, callback) {
	let xhttp = new XMLHttpRequest();
	xhttp.onreadystatechange = callback;
	xhttp.open("POST", file, true);
	xhttp.send("");
}

game.setcallback = function() {
	if(this.readyState < 4)
		return ;
	if(this.responseText == "Y") { // it is your turn to fire
		view.displayMessage("Yey! It is your turn!");
		return ;
	} else {						  // a shot has come
		this.func = game.handleShot;
		view.displayMessage("It's your turn now");
		this.func();
	}
}

game.initGame = function() {
	game.getPosition.process_clicks.removeCallbacks();
	view.displayMessage("Waiting for other player");
	let file_request = new String();
	parameters.player_field.forEach( (e) => {
		e.forEach( (el) => file_request += String(el) );
	}); 
	file_request = "/game?" + file_request;
	let but = document.getElementById("fireButton");
	but.onclick = game.shot;
	this.setShips(file_request, this.setcallback);
};

function main() {
	let but = document.getElementById("fireButton");
	but.onclick = game.getPosition.parseShip;
	game.getPosition.process_clicks.setCallbacks();
	document.onkeyup = (e) => { if(e.keyCode == 13) game.getPosition.parseShip(); };
	view.displayMessage("Hello Stranger! \n Enter all your ships below: ");
}

window.onload = main;
