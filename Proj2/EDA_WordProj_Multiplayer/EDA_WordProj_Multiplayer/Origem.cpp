#include <iostream>
#include <vector>
#include <fstream>
#include <string>  
#include <cctype>
#include <cstdlib>  // For system() function
#include <limits>
#include <windows.h>
#include <filesystem>
#include <algorithm>  
#include <random> 
#include <numeric>
#include <cmath>
#include <cctype>

//--------------------------------------------------------------------------------
using namespace std;
namespace fs = std::filesystem;

//================================================================================
// COLOR CODES / ANSI ESCAPE SEQUENCES
//================================================================================
// TEXT COLOR
#define NO_COLOR "\033[0m"
#define BLACK "\033[0;30m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define BROWN "\033[0;33m"
#define BLUE "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CYAN "\033[0;36m"
#define LIGHTGRAY "\033[0;37m"
#define DARKGRAY "\033[1;30m"
#define LIGHTRED "\033[1;31m"
#define LIGHTGREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define LIGHTBLUE "\033[1;34m"
#define LIGHTMAGENTA "\033[1;35m"
#define LIGHTCYAN "\033[1;36m"
#define WHITE "\033[1;37m"
// BACKGROUND COLOR
#define BLACK_B "\033[0;40m"
#define RED_B "\033[0;41m"
#define GREEN_B "\033[0;42m"
#define YELLOW_B "\033[0;43m"
#define BLUE_B "\033[0;44m"
#define MAGENTA_B "\033[0;45m"
#define CYAN_B "\033[0;46m"
#define WHITE_B "\033[1;47m"

// ===================================== Custom variables =====================================

typedef struct
{
	char lin;
	char col;
	char dir;
} WordPosition;

struct CharsPosition
{
	char lin;
	char col;
	char dir;

	// Operator overloading for the find function to work with the custom structure. 
	// Also, this does not work with typedef struc syntax from C, only with the newer C++ syntax
	bool operator==(const CharsPosition& other) const
	{
		if(lin == other.lin && col == other.col) // The line and column are enough for the comparison
		{
			return true;	
		}
		else
		{
			return false;
		}
	}
};

typedef struct
{
	WordPosition pos;
	string word;
} WordsOnBoard;

typedef struct
{
	vector<CharsPosition> pos;
	vector<char> charWord;
	int nLetters;
} CharsOnBoard;

typedef struct {
	vector<vector<char>> boardCells;
	int numLins;
	int numCols;
	vector<WordsOnBoard> wordsOnBoard; // Used fr�or reconstruction the board inside the code
	vector<CharsOnBoard> allChars; // Used to keep track of the letters left on the board
}BoardStruct;

struct Cell {
	char character;
	string color;
};

// ===================================== Board Class =====================================

class Board {
private:
	pair<string, vector<string>> folder_file;
	string folder;
	vector<string> txtFiles;
	BoardStruct boardStruct;
	
	pair<string, vector<string>> getTxtFilesInFolder();
	BoardStruct readBoardFile(const string& folder, const vector<string>& txtFiles);

public:
	Board()
	{
		// Initializing members in the constructor
		folder_file = getTxtFilesInFolder();
		folder = folder_file.first;
		txtFiles = folder_file.second;
		boardStruct = readBoardFile(folder, txtFiles);
	}

	BoardStruct& getBoardStruct();
	void showBoard(vector<CharsPosition> c);
	pair<int, CharsPosition> insertLetOnBoard(CharsOnBoard& charsOnHand, CharsOnBoard& charsOnBag);
};

// ===================================== Board Class functions =====================================

// Lists the files on the folder with the saved games
pair <string, vector<string>> Board::getTxtFilesInFolder() {

	vector<string> txtFiles;
	string folder;

	while (1)
	{
		cout << "\nChoose a folder with saved board games: ";
		cin >> folder;

		if (fs::exists(folder))
		{
			cout << "\nFiles listed: " << endl;
			for (const auto& entry : fs::directory_iterator(folder)) {
				if (entry.is_regular_file() && entry.path().extension() == ".txt") {
					txtFiles.push_back(entry.path().string());
					cout << entry.path().filename().string() << endl;
				}
			}
			break;
		}
		else
		{
			cout << "\nThe folder does not exist. Make sure the right path is indicated!" << endl;
		}
	}
	return make_pair(folder, txtFiles);
}

// Reads the file chosen by the users and retrieves the saved game board information to reconstruct the board 
BoardStruct Board::readBoardFile(const string& folder, const vector<string>& txtFiles)
// Although the function is big and with a lot of identations, it was meant this way in order to only loop through the tex
// file once to retrieve all the information regarding both the board and board characters, prioritizing program efficency
{
	bool validFile = false;
	BoardStruct boardStruct;
	string boardFile;

	while (!validFile)
	{
		// Check if file to read is valid
		cout << "\nText file: ";
		cin >> boardFile;
		boardFile = folder + "\\" + boardFile; // Concatenating the folder to the string written by the user to form the path to the file

		const auto& it = find(txtFiles.begin(), txtFiles.end(), boardFile);

		if (it != txtFiles.end())
		{
			validFile = true;
		}
		else
		{
			cout << "\nThe file chosen is not valid." << endl;
		}

		// If the file is valid it will be read
		if (validFile)
		{
			vector<string> fileTxtArray;
			ifstream inputFile(boardFile); //Open file
			if (inputFile.is_open())
			{
				string line;
				while (getline(inputFile, line))
				{
					fileTxtArray.push_back(line); // Read each line
				}

				inputFile.close(); // Close file
			}
			else
			{
				cout << "Error reading file." << endl;
			}

			// Board size variables
			int numCol;
			int numLin;
			bool posStrFound = false;

			// Board words and corresponding position variables:
			WordsOnBoard wordsOnBoard;
			WordPosition wordPos;
			bool beginWordRead = false;

			// Loop through each line on the txt file
			for (string& fileLine : fileTxtArray)
			{
				// Retrive the Board size information:
				if (!posStrFound)
				{
					string posStr = fileLine.substr(fileLine.find_first_of(":") + 1, fileLine.size());
					int x_pos = posStr.find_first_of("x");

					if (x_pos != string::npos) /*string::npos is used to represent "no position". If x_pos is not equal to string::npos, it means that the substring "x" was found in the string.*/
					{
						numLin = stoi(posStr.substr(0, x_pos)); /*First argument of substr : start position | Second argument : Length of the string evaluated*/
						numCol = stoi(posStr.substr(x_pos + 1)); /*If no second argument is given, the substr will start from the positivion given on the argument to the end of the string*/
						boardStruct.numLins = numLin;
						boardStruct.numCols = numCol;
						boardStruct.boardCells = vector<vector<char>>(numLin, vector<char>(numCol, '.')); /*Initializing the board cells*/
						posStrFound = true;
					}
				}

				// Retrieve the words on the Board with their corresponding positions and populates its cells:
				if (beginWordRead)
				{
					// Words on the board:
					auto it = remove_if(fileLine.begin(), fileLine.end(), ::isspace);
					fileLine.erase(it, fileLine.end()); /*Removing the spaces between the word and its corresponding position*/

					wordsOnBoard.word = fileLine.substr(3);
					int wordSize = wordsOnBoard.word.size();

					// Position Information:
					wordPos.lin = fileLine[0]; wordPos.col = fileLine[1]; wordPos.dir = fileLine[2];
					wordsOnBoard.pos = wordPos;

					// Populating the Board structure with the words on the file
					switch (wordPos.dir)
					{
					case 'H':
						 
						for (int c = 0; c < wordSize; c++)
						{
							boardStruct.boardCells[wordPos.lin - 'A'][wordPos.col - 'a' + c] = wordsOnBoard.word[c];
						}
						break;
					case 'V':
						for (int c = 0; c < wordSize; c++)
						{
							boardStruct.boardCells[wordPos.lin - 'A' + c][wordPos.col - 'a'] = wordsOnBoard.word[c];
						}
						break;
					}
					
					boardStruct.wordsOnBoard.push_back(wordsOnBoard);
					
				}

				int begin = fileLine.find("-"); // Finds the '-' char for on the next iteration start to populate the cells and save the words on the structure
				if (begin != string::npos)
				{
					beginWordRead = true;
				}

			}
		}
	}
	return boardStruct;
}

BoardStruct& Board::getBoardStruct() {
	return boardStruct;
}

// Prints the Board
void Board::showBoard(vector<CharsPosition> C)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	cout << endl;
	cout << BLUE << "  ";
	for (int j = 0; j < boardStruct.boardCells.at(0).size(); j++)
		cout << (char)('a' + j) << ' ';
	cout << endl;
	cout << NO_COLOR;

	for (int i = 0; i < boardStruct.boardCells.size(); i++)
	{
		cout << BLUE << (char)('A' + i) << ' ' << NO_COLOR;
		for (int j = 0; j < boardStruct.boardCells.at(i).size(); j++)
		{
			CharsPosition currentPos;
			currentPos.lin = 'A' + i;
			currentPos.col = 'a' + j;

			if (find(C.begin(), C.end(), currentPos) != C.end())
			{
				SetConsoleTextAttribute(hConsole, 2);
				cout << boardStruct.boardCells.at(i).at(j) << ' ';
				SetConsoleTextAttribute(hConsole, 7);
			}
			else
			{
				cout << boardStruct.boardCells.at(i).at(j) << ' ';
			}
		}
		cout << endl;
	}
	cout << endl;
}

pair<int, CharsPosition> Board::insertLetOnBoard(CharsOnBoard& charsOnHand, CharsOnBoard& charsOnBag)
{
	// Less identation is better, provides better readibility. 
	// For instance, GOOGLE engineers are asked to not have more than 3 identations in the code unless it can be justified
	// This is the reason to validate inputs this way

	bool validC = false;
	char C;

	while (!validC) // Checks for valid Letters on the Hand
	{
		cout << "Insert letter: ";
		cin >> C;
		C = toupper(C);

		const auto itL = find(charsOnHand.charWord.begin(), charsOnHand.charWord.end(), C);

		if (itL != charsOnHand.charWord.end()) // If the Letter in the Player's hand
		{
			validC = true;
		}
		else
		{
			cout << "Letter " << C << " is not a word on your hand!" << endl;
		}
	}

	bool validPos = false; 
	string P;
	CharsPosition cPos;
	while (!validPos) // Checks for valid Letter position on the Hand
	{
		cout << "Position: ";
		cin >> P;
		cPos.lin = P[0];
		cPos.col = P[1];

		const auto itPHand = find(charsOnHand.pos.begin(), charsOnHand.pos.end(), cPos);

		if (itPHand != charsOnHand.pos.end() && isupper(P[0]) && islower(P[1]))
		{
			const int idx = distance(charsOnHand.pos.begin(), itPHand);
			if (charsOnHand.charWord[idx] == C)
			{
				validPos = true;
			}
		}
		else
		{
			cout << "This position is invalid!" << endl;
		}
	}

	int points = 0;
	const auto itPHand = find(charsOnHand.pos.begin(), charsOnHand.pos.end(), cPos);
	const int idx = distance(charsOnHand.pos.begin(), itPHand);
	bool fnd = false;

	for (CharsOnBoard& ch : boardStruct.allChars) // Eliminating the letter from the hand 
	{
		for (int i = 0; i < ch.charWord.size(); i++)
		{
			if (ch.charWord[0] == C && ch.pos[0].col == cPos.col && ch.pos[0].lin == cPos.lin && !fnd)
			{
				ch.charWord.erase(ch.charWord.begin());
				ch.pos.erase(ch.pos.begin() + i);
				
				cPos.dir = '+';
				charsOnHand.charWord.erase(charsOnHand.charWord.begin() + idx);
				charsOnHand.pos.erase(charsOnHand.pos.begin() + idx);
				fnd = true;
			}
			else
			{
				if (!fnd)
				{
					cPos.dir = '-';
				}
			}
		}
	}

	if(fnd) // Checking for duplicates and eliminating them
	{ 
		for (CharsOnBoard& ch : boardStruct.allChars)
		{
			for (int i = 0; i < ch.charWord.size(); i++)
			{
				if (ch.charWord[i] == C && ch.pos[i].col == cPos.col && ch.pos[i].lin == cPos.lin)
				{
					ch.charWord.erase(ch.charWord.begin() + i);
					ch.pos.erase(ch.pos.begin() + i);
				}
			}

			auto it = find_if(boardStruct.allChars.begin(), boardStruct.allChars.end(), [](const CharsOnBoard& ch)
				{
					return ch.charWord.size() == 0;
				});

			if (it != boardStruct.allChars.end())
			{
				points++;
				size_t index = distance(boardStruct.allChars.begin(), it);
				boardStruct.allChars.erase(boardStruct.allChars.begin() + index);
			}

		}
		
	}

	return make_pair(points, cPos);
}


// ===================================== Bag Class =====================================

class Bag {
private:
	CharsOnBoard constructBag(BoardStruct& boardStruct);
	CharsOnBoard charsOnBag;

public:
	Bag(BoardStruct& boardStruct) 
	{
		// Initializing members in the constructor
		charsOnBag = constructBag(boardStruct);
	}
	
	CharsOnBoard& getBag();

	void shuffle(CharsOnBoard& charsOnBoard);
	bool isEmpty(const CharsOnBoard& charsOnBag);
};

// ===================================== Bag Class functions =====================================

CharsOnBoard Bag::constructBag(BoardStruct& boardStruct)
{
	// Word characters and corresponding position variables
	CharsOnBoard charsOnBoard;
	CharsPosition charsPosition;

	for (const WordsOnBoard& words : boardStruct.wordsOnBoard)
	{
		CharsOnBoard wordChars;
		CharsPosition charsPos;
		int wordSize = words.word.size();

		charsOnBoard.charWord.reserve(wordSize); /* Reserve pre - allocates memory to the vector improving the efficiency of the code */
		charsOnBoard.pos.reserve(wordSize);

		// Decomposing the board words into each character and its position
		switch (words.pos.dir)
		{
		case 'H': // Horizontal direction
			for (int c = 0; c < wordSize; c++)
			{
				// Retrieve each character information from each word on the board
				charsPosition.lin = words.pos.lin; // Character line
				charsPosition.col = words.pos.col + c; // Character column

				// Checks whether the character is specific to a word or belongs to two words. If so, the + char references that in the char position
				if (words.pos.lin - 'A' > 0 && words.pos.lin - 'A' < boardStruct.numLins &&
					(boardStruct.boardCells[charsPosition.lin - 'A' - 1][charsPosition.col - 'a'] == '.' ||
						boardStruct.boardCells[charsPosition.lin - 'A' + 1][charsPosition.col - 'a'] == '.'))
				{
					charsPosition.dir = 'H';
				}
				else if (words.pos.lin - 'A' == 0 && /* Upper border */
					boardStruct.boardCells[charsPosition.lin - 'A' + 1][charsPosition.col - 'a'] == '.')
				{
					charsPosition.dir = 'H';
				}
				else if (words.pos.lin - 'A' == boardStruct.numLins - 1 && /* Lower border */
					boardStruct.boardCells[charsPosition.lin - 'A' - 1][charsPosition.col - 'a'] == '.')
				{
					charsPosition.dir = 'H';
				}
				else
				{
					charsPosition.dir = '+';
				}

				auto it = find(charsOnBoard.pos.begin(), charsOnBoard.pos.end(), charsPosition);
				if (it == charsOnBoard.pos.end()) // Does not add duplicates
				{
					charsOnBoard.charWord.push_back(words.word[c]); // Character
					charsOnBoard.pos.push_back(charsPosition); // Position
				}

				wordChars.charWord.push_back(words.word[c]);
				charsPos.col = charsPosition.col;
				charsPos.lin = charsPosition.lin;
				charsPos.dir = charsPosition.dir;
				wordChars.pos.push_back(charsPos);

			}
			break;

		case 'V': // Vertical direction
			for (int c = 0; c < wordSize; c++)
			{
				// Reconstruct board cells for each word
				boardStruct.boardCells[words.pos.lin - 'A' + c][words.pos.col - 'a'] = words.word[c];

				// Retrieve each character information from each word on the board
				charsPosition.lin = words.pos.lin + c; // Character line
				charsPosition.col = words.pos.col; // Character column

				// Checks whether the character is specific to a word or belongs to two words. If so, the + char references that in the char position
				if (words.pos.col - 'a' > 0 && words.pos.col - 'a' < boardStruct.numCols &&
					(boardStruct.boardCells[charsPosition.lin - 'A'][charsPosition.col - 'a' - 1] == '.' ||
						boardStruct.boardCells[charsPosition.lin - 'A'][charsPosition.col - 'a' + 1] == '.'))
				{
					charsPosition.dir = 'V';
				}
				else if (words.pos.col - 'a' == 0 && /* Left border */
					boardStruct.boardCells[charsPosition.lin - 'A'][charsPosition.col - 'a' + 1] == '.')
				{
					charsPosition.dir = 'V';
				}
				else if (words.pos.col - 'a' == boardStruct.numCols && /* Right border */
					boardStruct.boardCells[charsPosition.lin - 'A'][charsPosition.col - 'a' - 1] == '.')
				{
					charsPosition.dir = 'V';
				}
				else
				{
					charsPosition.dir = '+';
				}

				auto it = find(charsOnBoard.pos.begin(), charsOnBoard.pos.end(), charsPosition);
				if (it == charsOnBoard.pos.end())// Does not add duplicates
				{
					charsOnBoard.charWord.push_back(words.word[c]); // Character
					charsOnBoard.pos.push_back(charsPosition); // Position
				}

				wordChars.charWord.push_back(words.word[c]);
				charsPos.col = charsPosition.col;
				charsPos.lin = charsPosition.lin;
				charsPos.dir = charsPosition.dir;
				wordChars.pos.push_back(charsPos);

			}
			break;
		}

		boardStruct.allChars.push_back(wordChars);
	}
	charsOnBoard.nLetters = charsOnBoard.charWord.size();
	return charsOnBoard;
}

CharsOnBoard& Bag::getBag() {
	return charsOnBag;
}

void Bag::shuffle(CharsOnBoard& charsOnBoard) //Shuffling using the Fisher-Yates shuffle algorithm
{
	random_device rd;
	mt19937 g(rd());

	vector<int> indices(charsOnBoard.charWord.size());
	iota(indices.begin(), indices.end(), 0); /* Creates a vector of incremental int values starting from 0 */
	std::shuffle(indices.begin(), indices.end(), g);

	// Swap elements based on shuffled indices
	for (int i = 0; i < indices.size(); ++i) {
		if (i != indices[i]) {
			swap(charsOnBoard.pos[i], charsOnBoard.pos[indices[i]]);
			swap(charsOnBoard.charWord[i], charsOnBoard.charWord[indices[i]]);
		}
	}
}

bool Bag::isEmpty(const CharsOnBoard& bag) { // To check if the bag is empty
	return bag.charWord.empty();
}

// ===================================== Hand Class =====================================

class Hand
{
private:
	CharsOnBoard handLetters;

public:
	Hand(const int numLetters, CharsOnBoard& bag, bool createHand)
	{
		handLetters = getLettersFromBag(numLetters, bag, createHand);
	}
	CharsOnBoard& getHand();
	CharsOnBoard& getLettersFromBag(const int handSize, CharsOnBoard& bag, bool createHand);
	void showHand();
	void switchHand(CharsOnBoard& bag);
	bool playableHand(BoardStruct& boardStruct);
};

// ===================================== Hand Class functions =====================================

CharsOnBoard& Hand::getLettersFromBag(const int numLetters, CharsOnBoard& bag, bool createHand)
{
	handLetters.charWord.reserve(numLetters);
	handLetters.pos.reserve(numLetters);
	if (createHand)
	{
		handLetters.nLetters = numLetters;
		for (int let = 0; let < numLetters; let++)
		{
			// Populating the Hand
			handLetters.charWord.push_back(bag.charWord[let]);
			handLetters.pos.push_back(bag.pos[let]);
		}
	}
	else
	{
		for (int let = 0; let < numLetters; let++)
		{
			// Populating the Hand
			handLetters.charWord.push_back(bag.charWord[let]);
			handLetters.pos.push_back(bag.pos[let]);
		}
	}

	// Remove the specific Letters from the Bag reference
	bag.charWord.erase(bag.charWord.begin(), bag.charWord.begin() + numLetters);
	bag.pos.erase(bag.pos.begin(), bag.pos.begin() + numLetters);
	bag.nLetters -= numLetters;
	return handLetters;
}

CharsOnBoard& Hand::getHand()
{
	return handLetters;
}


void Hand::showHand()
{
	cout << LIGHTRED << "Hand: " << NO_COLOR;
	for (int l = 0; l < handLetters.charWord.size(); l++)
	{
		cout << handLetters.charWord[l] << ": " << handLetters.pos[l].lin << handLetters.pos[l].col << " | ";
	}
	cout << "\n" << endl;
}

void Hand::switchHand(CharsOnBoard& bag)
{
	// Only one letter can be switched at a time. 
	// This way is more robust to wrong inputs and simple to implement when deconstructing the string used with 2 Chars 

	char replaceL;

	cout << "Letter to switch: ";
	cin >> replaceL;

	replaceL = toupper(replaceL);

	CharsOnBoard aux;
	const auto letIt = find(handLetters.charWord.begin(), handLetters.charWord.end(), replaceL);
	const int index = distance(handLetters.charWord.begin(), letIt);

	if (letIt != handLetters.charWord.end())
	{
		// Swapping letter with bag
		swap(bag.charWord[bag.charWord.size() - 1], handLetters.charWord[index]);
		swap(bag.pos[bag.pos.size() - 1], handLetters.pos[index]);
	}
	else
	{
		cout << "\nInvalid Letter!\n" << endl;
	}
}

bool Hand::playableHand(BoardStruct& boardStruct) // Ensures there are playable Letters on the Hand
{
	for (CharsOnBoard& w : boardStruct.allChars)
	{

		for (int c = 0; c < handLetters.charWord.size(); c++)
		{
			if (w.charWord[0] == handLetters.charWord[c] && w.pos[0].col == handLetters.pos[c].col && w.pos[0].lin == handLetters.pos[c].lin)
			{
				return true;
			}
		}


	}
	return false;
}

// ===================================== Player Class =====================================

class Player
{
private:
	int id;
	string name;
	int points;
public:
	Player(int id_, string name_, int pt_) // Constructor
	{	
		setId(id_);
		setName(name_); 
		points = pt_;
		cout << "Player " << id_ << " named " << name_ << " has been created!" << endl;
	}

	int getId();
	void setId(int id);

	string getName();
	void setName(string name);

	void addPoints(int point);
	int& getPoints();
};

// ===================================== Player class functions =====================================

int Player::getId()
{
	return id;
}

void Player::setId(int id_)
{
	id = id_;
}

string Player::getName()
{
	return name;
}

void Player::setName(string name_)
{
	name = name_;
}

void Player::addPoints(int pt)
{
	points += pt;
}

int& Player::getPoints()
{
	return points;
}

// ===================================== Non-class functions =====================================

// Converts all characters of �s� to lowercase
void tolowerStr(string& s)
// Because a pointer is used, a return is unecessary since the changes are occuring to the original variable
{
	// The added condition enables the use of words with random uppercase and lowercase chars
	for (auto& c : s) if (!islower(c)) c = tolower(c);
}
//================================================================================
// Converts all characters of �s� to uppercase
void toupperStr(string& s)
// Because a pointer is used, a return is unecessary since the changes are occuring to the original variable
{
	// The added condition enables the use of words with random uppercase and lowercase chars
	for (auto& c : s) if (!isupper(c)) c = toupper(c);
}

int read_nPlayers()
{
	// The user is asked the number of players that will play the game simultaneously

	int n_players;
	int min_n_players = 2; int max_n_players = 4; // Hardcoded range of players
	bool valid_n_players = false;

	do
	{
		cout << "\nNumber of players (2-4): ";
		cin >> n_players;

		if (n_players >= min_n_players && n_players <= max_n_players)
		{
			valid_n_players = true;
		}
		else
		{
			cout << "Error: Number of players is invalid!" << endl;
		}

	} while (!valid_n_players); // The loop will run until a valid number of players is inserted

	return n_players;
}

int numLettersToEachPlayer(int nPlayers, int numLetters)
{
	string answ;
	int suggestHandSize = static_cast<int>(ceil(0.5 * numLetters / nPlayers));
	int maxHandSize = static_cast<int>(ceil(0.8 * numLetters / nPlayers));
	
	bool validHand = false;
	int handSize = 0;

	do
	{
		cout << "Suggested number of letters to distribute for each Player: " << suggestHandSize;
		cout << "\nDo you want to change (Yes/No): ";
		cin >> answ;

		toupperStr(answ);

		if (answ == "YES")
		{
			while (!validHand)
			{
				cout << "Hand size (2 - " << maxHandSize << "): ";
				cin >> handSize;
				if (handSize >= 2 && handSize <= maxHandSize)
				{
					validHand = true;
				}
			}
		}
		else
		{
			handSize = suggestHandSize;
		}

	} while (answ != "YES" && answ != "NO");
	return handSize;
}

char readOption() { // Reads user game option and returns the choosen one
	cout << CYAN << "H - Help | " << "I - Insert words | " << "P - Pass play | "  << "S - Swap | " << "Q - Quit" << endl << NO_COLOR << "\nOption: " ;

	char inserted;
	cin >> inserted;

	if (!isupper(inserted))
	{
		inserted = toupper(inserted);
	}

	return inserted;
}

void showHelp() {
	cout << CYAN << "\n\n ------------------ Help ------------------\n" << endl;

	cout << CYAN << "I - " << WHITE << "The letter will be placed according to the selected position on the Board cell.\n"
			"If Aa will be chosen, the letter will be placed in the row A and column a. The letters\n"
			"must be placed according to the order of the word. The placed letter, if valid,\n"
			"will be displayed in red. If the Player completes a word he will rreceive 1 point.\n"
			"In case the Player completes 2 words with one letter, he will receive 2 points.\n\n";

	cout << CYAN << "P - " << WHITE << "The play will be passed.\n\n";
	cout << CYAN << "S - " << WHITE << " This option will swap a letter of the Players Hand of letters with the bag of \n\n"
		    "letters containing all letters on the board. \n\n";
	cout << CYAN << "Q - " << WHITE << " The Player chooses to quit the game.\n" << endl;
	
	cout << "--------------------------------------------\n" << WHITE << endl;
}

// =================================================== Main =================================================

int main()
{
	cout << "================ Multiplayer Board Game ================" << endl;
	CharsPosition col;
	vector<CharsPosition> vclCh;

	// Initializing BOARD:
	Board board;
	BoardStruct& boardStruct = board.getBoardStruct();
	board.showBoard(vclCh);

	//Initializing BAG:
	Bag bag(boardStruct);
	CharsOnBoard& charsOnBag = bag.getBag();
	bag.shuffle(charsOnBag);

	// Intializing PLAYERS and HANDS:
	int nPlayers = read_nPlayers();
	int handSize = numLettersToEachPlayer(nPlayers, charsOnBag.nLetters);

	cout << "\n -------- Player and Hand creation --------\n" << endl;
	vector<Player> players;
	players.reserve(nPlayers); /* Memory pre-allocation */

	vector<Hand> hands;
	hands.reserve(nPlayers);


	// This loop will initilize the players and construct their corresponding hands, mapped through the loop iterator
	for (int id = 1; id <= nPlayers; id++) 
	{
		string name;
		cout << "Player " << id << " | " << "Insert name: ";
		cin >> name;
		Player pl(id, name, 0);
		players.push_back(pl);

		Hand hand(handSize, charsOnBag, true);
		hands.push_back(hand);
		CharsOnBoard& charsOnHand = hands[id - 1].getHand();
		hands[id - 1].showHand();
	}
	
	// Game loop:

	cout << " ------------- Game starting --------------\n" << endl;
	board.showBoard(vclCh);
	pair<int, CharsPosition> info;
	CharsPosition clCh;
	int point;

	while (nPlayers >= 2 && boardStruct.allChars.size() > 0)
	{
		for (int pl = 0; pl < nPlayers; pl++)
		{
			int gameSize = 2;
			cout << RED << "Turn - Player " << players[pl].getId() << ": " << GREEN << players[pl].getName() << NO_COLOR << endl;
			for (int sw = 0; sw < gameSize; sw++)
			{
				CharsOnBoard& hd = hands[pl].getHand();
				int& pl_point = players[pl].getPoints();

				bool exitOuterLoop = false;
				char opt = readOption();
				switch (opt)
				{
				case 'H':
					showHelp();
					gameSize++;
					break;
				case 'I':
					hands[pl].showHand();
					if (hands[pl].playableHand(boardStruct))
					{
						info = board.insertLetOnBoard(hd, charsOnBag);
						point = info.first;
						col = info.second;
						if (col.dir != '-')
						{
							vclCh.push_back(col);
							players[pl].addPoints(point);
							
							if (!bag.isEmpty(charsOnBag))
							{
								hands[pl].getLettersFromBag(1, charsOnBag, false);
							}
							if (point > 0)
							{
								cout << LIGHTRED << "Scored points: " << NO_COLOR << point << endl;
							}
						}
						else 
						{
							gameSize++;
							cout << "This Letter does not respect the sequential order of the words! Try again." << endl;
						}
					}
					else
					{
						gameSize++;
						cout << RED << "Your HAND does not contain valid Letters. Your options are to PASS or SWAP!"<< NO_COLOR <<endl;
					}
					board.showBoard(vclCh);
					break;
				case 'P':
					exitOuterLoop = true;
					break;
				case 'S':
					if (!bag.isEmpty(charsOnBag))
					{
						hands[pl].showHand();
						hands[pl].switchHand(charsOnBag);
						cout << LIGHTRED <<"New " << NO_COLOR;
						hands[pl].showHand();
						bag.shuffle(charsOnBag);
					}
					else
					{
						cout << "Bag is empty!" << endl;
					}
					break;
				case 'Q':
					exitOuterLoop = true;
					cout << "\nPlayer " << players[pl].getId() << ": " << players[pl].getName() << " has quit the game!" << endl;
					players.erase(players.begin() + pl);
					hands.erase(hands.begin() + pl);
					nPlayers -= 1;
					break;
				}

				if (exitOuterLoop)
				{
					break;
				}

			}
		}
	}
	//End of the game
	vector<int> finalpoints;
	finalpoints.reserve(nPlayers);
	for (int pll = 0; pll < nPlayers; pll++)
	{	
		finalpoints.push_back(players[pll].getPoints());
		
	}
	
	int dis;
	auto maxElement = max_element(finalpoints.begin(), finalpoints.end());
	if (maxElement != finalpoints.end())
	{
		dis = distance(finalpoints.begin(), maxElement);
	}
	else
	{
		dis = 0;
	}

	cout << RED << "========== Player (Id:" << players[dis].getId() << ") " << players[dis].getName() << " has won the game with " << finalpoints[dis] << " points! ==========" << NO_COLOR << endl;

	return 0;	
}

