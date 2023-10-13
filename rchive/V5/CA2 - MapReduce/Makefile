CXX = g++
CXXFLAGS = -std=c++17

SRC_DIR = src
OBJ_DIR = obj


OUT_CAS = ClubsAgeStats.out
OUT_COUNTRY = country.out
OUT_CLUB = club.out
OUT_POS = position.out

VPATH = $(SRC_DIR)


all: $(OUT_CAS) $(OUT_COUNTRY) $(OUT_CLUB) $(OUT_POS)

$(OUT_CAS): $(OBJ_DIR)/ClubsAgeStats.o $(OBJ_DIR)/logger.o $(OBJ_DIR)/csv.o $(OBJ_DIR)/strutils.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OUT_COUNTRY): $(OBJ_DIR)/country.o $(OBJ_DIR)/logger.o $(OBJ_DIR)/strutils.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OUT_CLUB): $(OBJ_DIR)/club.o $(OBJ_DIR)/logger.o $(OBJ_DIR)/csv.o $(OBJ_DIR)/strutils.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OUT_POS): $(OBJ_DIR)/position.o $(OBJ_DIR)/logger.o $(OBJ_DIR)/csv.o $(OBJ_DIR)/strutils.o
	$(CXX) $(CXXFLAGS) $^ -o $@



$(OBJ_DIR)/ClubsAgeStats.o: $(SRC_DIR)/main.cpp $(SRC_DIR)/logger.hpp $(SRC_DIR)/colorprint.hpp $(SRC_DIR)/csv.hpp \
							$(SRC_DIR)/consts.hpp $(SRC_DIR)/strutils.hpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/country.o: $(SRC_DIR)/country.cpp $(SRC_DIR)/logger.hpp $(SRC_DIR)/colorprint.hpp $(SRC_DIR)/strutils.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/club.o: $(SRC_DIR)/club.cpp $(SRC_DIR)/logger.hpp $(SRC_DIR)/colorprint.hpp $(SRC_DIR)/csv.hpp \
					$(SRC_DIR)/consts.hpp $(SRC_DIR)/strutils.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/position.o: $(SRC_DIR)/position.cpp $(SRC_DIR)/logger.hpp $(SRC_DIR)/colorprint.hpp $(SRC_DIR)/csv.hpp \
						$(SRC_DIR)/consts.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/logger.o: $(SRC_DIR)/logger.cpp $(SRC_DIR)/logger.hpp $(SRC_DIR)/colorprint.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/csv.o: $(SRC_DIR)/csv.cpp $(SRC_DIR)/csv.hpp $(SRC_DIR)/strutils.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/strutils.o: $(SRC_DIR)/strutils.cpp $(SRC_DIR)/strutils.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)



.PHONY: all clean

clean:
	rm -f $(OBJ_DIR)/*.o ./*.out
