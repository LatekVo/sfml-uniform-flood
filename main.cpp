#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

#include <array>
#include <iostream>
#include <queue>
#include <chrono>
#include <thread>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Config.hpp>
#include <SFML/Window.hpp>

const int V_AM = 6;

//yes, this is a useless, avoidable bodge
std::array<std::array<std::array<bool, V_AM>, 50>, 50> *p_global_map = 0;
std::array<std::array<sf::RectangleShape, 50>, 50> *p_global_tileMap = 0;

int frame = 0;

void waitFrame() { 
	std::cout << "frame " << frame++ << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(35));
};
//NOTE: after reworking seemingly unrelated fill function, 
//		the first fill function started behaving in a different (not worse, different) fashion

// using 3d array was a absurdly dumb, i should know better by now, 
// it's too late to change without ripping half of the code

void c_room(int x, int y, int x_d, int y_d, std::array<std::array<std::array<bool, V_AM>, 50>, 50> *map) {
	if (x > x_d) {
		int tmp = x;
		x = x_d;
		x_d = tmp;
	}
	if (y > y_d) {
		int tmp = y;
		y = y_d;
		y_d = tmp;
	}
	
	for (int i = x; i <= x_d; i++) {
		(*map)[i][y][3] = 1;
		(*map)[i][y_d][3] = 1;
	} 
	for (int i = y; i <= y_d; i++) {
		(*map)[x][i][3] = 1;
		(*map)[x_d][i][3] = 1;
	}
}

void apply(std::array<std::array<std::array<bool, V_AM>, 50>, 50> *map) { 
//		   std::array<std::array<sf::RectangleShape, 50>, 50> &tileMap) {
		
	for (int y = 0; y < 50; y++) {
		for (int x = 0; x < 50; x++) {
			
			sf::Color color(0,0,0, 255);

			if ((*map)[x][y][3] == 1)
				//std::cout << '#';
				color = sf::Color(0x33, 0x36, 0x52);

			else if ((*map)[x][y][0] == 1)
				//std::cout << '%';
				color = sf::Color(0xfa, 0xd0, 0x2c);
			
			else if ((*map)[x][y][4] == 1) {
				if ((*map)[x][y][5] == 1)
					//std::cout << '.';
					color = sf::Color(0xe9, 0xea, 0xec);
				else
					//std::cout << 'O';
					color = sf::Color(0x28, 0x21, 0x20);
			}
			
			else if ((*map)[x][y][2] == 1)
				//std::cout << '~';
				color = sf::Color(0x90, 0xad, 0xc6);
			
			else if ((*map)[x][y][1] == 1)
				//std::cout << '.';
				color = sf::Color(0xe9, 0xea, 0xec);
			else { 
				//std::cout << '~'; //black
				color = sf::Color(0x98, 0xd7, 0xc2);
			}
			
			(*p_global_tileMap)[x][y].setFillColor(color);
			//std::cout << " ";
		}
		//std::cout << "\n";
	}
	//std::cout.flush();
	//std::this_thread::sleep_for(std::chrono::milliseconds(30));
}

void draw(sf::RenderWindow &window) {
	//DRAWING - don't delete
	window.clear(sf::Color::Black);
	for (int y = 0; y < 50; y++) {
		for (int x = 0; x < 50; x++) {
			window.draw((*p_global_tileMap)[x][y]);
		}
	}
	window.display();
}

class Door {
	//state is on map
	int id;
	int x, y;
	sf::RenderWindow *win;
	void toggleFill(std::array<std::array<std::array<bool, V_AM>, 50>, 50> &map) { 
		
		//NOTE: this function is partially redundant, SEE: main()::fill();
	
		//NOTE: first look for water source, filling the room with traces
		bool feMode = map[x][y][5];
		bool isWater = false;
		//this is inside so i shouldn't worry about the border
		if((map[x+1][y][2] |
			map[x-1][y][2] |
			map[x][y+1][2] |
			map[x][y-1][2] ) == 1) {

			isWater = true;
		}
		
		//update every block
		map[x+1][y][0] = 1; 
		map[x-1][y][0] = 1;
		map[x][y+1][0] = 1;
		map[x][y-1][0] = 1;
		
		/* 0 - update
		 * 1 - insides
		 * 2 - water
		 * 3 - walls
		 * 4 - doors
		 * 5 - door_state (0 close, 1 open)
		 *	
		 *	(x) >= 0 && (y) >= 0 &&
		 *	(x) < 50 && (y) < 50 &&
		 */
		
		//search area will be incremented by one in each direction
		bool isDone = false;
		int x_min = 1, x_max = 1, y_min = 1, y_max = 1;
		while (!isDone) {
			isDone = true;
			for (int s_y = y - y_min; s_y <= y_max; s_y++) {
				for (int s_x = x - x_min; s_x <= x_max; s_x++) {
						
					if (map[s_x][s_y][0] == 0 &&  		//update
						map[s_x][s_y][1] == 1 &&  		//insides
						map[s_x][s_y][2] == !isWater && //water (!isWater, if there is water, update where there isn't)
						map[s_x][s_y][3] == 0 &&  		//walls
					   (map[s_x][s_y][4] == 0 || 		//doors
						map[s_x][s_y][5] == 1)) {		//state (1 = open)
						 
							isDone = false;
							map[s_x+1][s_y][0] = 1; // ejasoned update
							map[s_x-1][s_y][0] = 1; // ejasoned update
							map[s_x][s_y+1][0] = 1; // ejasoned update
							map[s_x][s_y-1][0] = 1; // ejasoned update
							
							map[s_x]  [s_y][0] = 0; //update deletion
							map[s_x][s_y][2] = isWater; //water
					
					}
				}
			}

			x_min++;
			x_max++;
			y_min++;
			y_max++;
			std::cout << "Door::toggleFill(): enlarged scanning area." << std::endl;
			
			//border controll
			if (x - x_min < 1) {
				x_min--;
				std::cout << "Door::toggleFill(): fixed scanning area." << std::endl;
			} if (x + x_max >= 48) {
				x_max--;
				std::cout << "Door::toggleFill(): fixed scanning area." << std::endl;
			} if (y - y_min < 1) {
				y_min--;
				std::cout << "Door::toggleFill(): fixed scanning area." << std::endl;
			} if (y + y_max >= 48) {
				y_max--;
				std::cout << "Door::toggleFill(): fixed scanning area." << std::endl;
			}
			//NOTE: REACHED MAX SIZE
		}
	}
public:
	Door(int idd, int xx, int yy, sf::RenderWindow *win) {
		id = idd;
		x = xx;
		y = yy;
	}
	void toggle(std::array<std::array<std::array<bool, V_AM>, 50>, 50> &map) {
		map[x][y][5] = !map[x][y][5];

		toggleFill(map);
	}
};

int main() {

	//													  Title + Close
	sf::RenderWindow window(sf::VideoMode(1000, 1000), "My window", 1 | 4);
	window.setVerticalSyncEnabled(false);
	window.setFramerateLimit(60);
	
	//this should have just been a struct, too bad
	// map {} didn't work
	std::array<std::array<std::array<bool, V_AM>, 50>, 50> map;
	std::array<std::array<sf::RectangleShape, 50>, 50> tileMap;
	p_global_map = &map;
	p_global_tileMap = &tileMap;
	for (int y = 0; y < 50; y++) {
		for (int x = 0; x < 50; x++) {
			tileMap[x][y] = sf::RectangleShape(sf::Vector2f(20, 20));
			tileMap[x][y].setPosition(20*x, 20*y);			
			for (int i = 0; i < V_AM; i++) {
				map[x][y][i] = 0;
			}
		}
	}
	/* 0 - update
	 * 1 - insides
	 * 2 - water
	 * 3 - walls
	 * 4 - doors
	 * 5 - door_state (0 close, 1 open)
	 */

	c_room(20, 20, 30, 30, &map);
	map[20][25][3] = false;// wall
	map[20][25][4] = true; // door

	//separating ins and outs
	map[0][0][0] = true; //update


	auto fill_spot = [&map](int x, int y, int x_off, int y_off, 
					  std::array<std::array<std::array<bool, V_AM>, 50>, 50> *altMap = 0, bool opnCls = 0) {
		/*
		std::cout <<
			"states of " << &map << ": \n" <<
			"coords: " << x+x_off << " " << y+y_off << "\n" <<
			map[x+x_off][y+y_off][0] << " " <<
			map[x+x_off][y+y_off][4] << " " <<
			map[x+x_off][y+y_off][3] << " " <<
			std::endl;
		*/

		//NOTE: this function may have redundant functions, SEE: Door::toggleFill();

		//slower but more maintainable
		std::array<std::array<std::array<bool, V_AM>, 50>, 50> *m;
		if (altMap)
			m = altMap;
		else
			m = &map;

		if ((x + x_off) >= 0  && (y + y_off) >= 0  &&
			(x + x_off) < 50 && (y + y_off) < 50 &&
			(*m)[x+x_off][y+y_off][0] == 0 &&
			(*m)[x+x_off][y+y_off][2] == opnCls &&
			(*m)[x+x_off][y+y_off][3] == 0 &&
			((*m)[x+x_off][y+y_off][4] == 0 ||
			 (*m)[x+x_off][y+y_off][5] == 1)) {
			 
			 (*m)[x+x_off][y+y_off][0] = 1; //update
			 (*m)[x+x_off][y+y_off][2] = !opnCls; //water
			 //std::cout << "updated! " << x+x_off << " " << y+y_off << std::endl;
		}
		//yes, this is awful
	};

	bool isDone = false;
	while (!isDone) {
		isDone = true;
		auto mapCpy = map; // for visuals only
		for (int y = 0; y < 50; y++) {
			for (int x = 0; x < 50; x++) {
				//i can't set current based on others, i can't tell wether i should disable an update or not
				if (map[x][y][0] == 1) {
					isDone = false;
					fill_spot(x, y, 0, 1 , &mapCpy);
					fill_spot(x, y, 1, 0 , &mapCpy);
					fill_spot(x, y, 0, -1, &mapCpy);
					fill_spot(x, y, -1, 0, &mapCpy);
					mapCpy[x][y][0] = 0;
					
					//draw(&map);
					//std::cout << "wave at: " << x << " " << y << std::endl;
				}
			}
		}
	
		apply(&mapCpy);

		sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
	
		window.clear(sf::Color::Black);

		for (int y = 0; y < 50; y++) {
			for (int x = 0; x < 50; x++) {
				window.draw(tileMap[x][y]);
			}
		}
		window.display();

		waitFrame();

		map = mapCpy;
	}

	for (int y = 0; y < 50; y++) {
		for (int x = 0; x < 50; x++) {
			if (map[x][y][2] == 0 && map[x][y][3] == 0)
				map[x][y][1] = 1;
		}
	}
	
	apply(&map);		
	
	for (int y = 0; y < 50; y++) {
		for (int x = 0; x < 50; x++) {
			window.draw(tileMap[x][y]);
		}
	}

	waitFrame();
	window.display();

	//VIBECHECK above code in main() works correctly

	int d_idx = 0;
	std::queue<Door> doorList;
	//indexing doors
	for (int y = 0; y < 50; y++) {
		for (int x = 0; x < 50; x++) {
			if (map[x][y][4] == 1) {
				Door newDoor(d_idx++, x, y, &window);

				doorList.push(newDoor);
			}
		}
	}

	while(window.isOpen()) {        
		sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

		//cycle through doors
		doorList.front().toggle(map);
		
		doorList.push(doorList.front());
		doorList.pop();
	
		doorList.front().toggle(map);

		//DRAWING - don't delete
		window.clear(sf::Color::Black);
		for (int y = 0; y < 50; y++) {
			for (int x = 0; x < 50; x++) {
				window.draw(tileMap[x][y]);
			}
		}
		window.display();
		waitFrame();

	}

	return 0;
}
