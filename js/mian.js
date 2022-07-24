
let view = {
	displayMessage: function(msg) {
		var messageArea = document.getElementById("message");
		messageArea.innerHTML = msg;
	},
	displayCell(what, whose) {},
	clearField(whose) {}
};


let parameters = {
	player_field: [],
	oponent_field: [],
	ship_sizes: new Map([
	// first number is ship length, and second amount of such ships
		[1, 3],
		[2, 2],
		[3, 2],
		[4, 1],
		[5, 1],
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

parameters.ship_sizes.forEach((val) => parameters.ships_count += val);

let game = {
	getPosition: {
		last_length: 0,
		last_coords: [],
		count_ships_remain: parameters.ships_count,
		ships_remain: parameters.ships_sizes,
		parseShip() {},
		validateInput(coords) {},
		correctPosition(coords) {}, 
		setData() {}, 
		setPicture() {},
	},
	initGame() {},
};

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

	if(this.ships_remain.get(this.last_length) == 0) // ships of such length are already entered
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
	let Input = document.getElementById("guessInput");
	let coords = Input.value;
	Input.value = "";
	if(!game.getPosition.validateInput(coords))
		return ;
	
	// set ship
	game.getPosition.setData(game.getPosition.last_coords);

	// set pictures
	game.getPosition.setPicture();
}


function main() {
	let but = document.getElementById("fireButton");
	but.onclick = game.getPosition.parseShip;
}

window.onload = main;
