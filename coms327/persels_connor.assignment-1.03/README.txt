README

ABOUT:
	Supposed to print the map and the pathing maps for the hiker and the rival.

	I commented out a few chunks of code to disable some mechanics from 1.02 that were not necessary for 1.03.

ISSUES:
	I implemented placing the character randomly on a path, and I believe the pathing for both hiker and rival should be
	correct, but I am not happy with the way I implemented the code. I recognize that duplicating the Dijkstra algorithm
	more than once just between the rival and hiker is an extreme code smell, but I ran out of time towards the end
	and realized too late that to pass an 2d integer array I'd have to do pointer math that I did not have time for.

	The PC tends to spawn to the left side of the map. This is because I go to a random point when the map is made and
	I go from that point to the right until I hit a path. If I don't hit a path, I find a new point and repeat. This means
	that in most cases I end up with a PC character that tends to be closer to the left side of the map.

	I hope the working submission is enough for grading purposes.

ACCREDITATION:
	Everything up to assignment 1.03 was used from the answer code of assignment 1.02.

	I wrote everything that was not otherwise written from 1.02.

CONTACT:
	Connor Persels - cpersels@iastate.edu