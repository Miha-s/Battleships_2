
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
	
};

parameters.ship_sizes.forEach((val) => parameters.ships_count += val);

let game = {
	getPosition() {},
	initGame() {},
};

function main() {
	game.getPosition();
	game.initGame();
}
