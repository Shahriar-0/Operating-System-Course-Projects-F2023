CC = gcc
CFLAGS = 

SRC_DIR = src
OBJ_DIR = obj


OUT_CLIENT = client.out
OUT_SERVER = server.out

VPATH = $(SRC_DIR)


all: $(OUT_CLIENT) $(OUT_SERVER)

$(OUT_CLIENT): $(OBJ_DIR)/client.o $(OBJ_DIR)/logger.o $(OBJ_DIR)/utils.o
	$(CC) $(CFLAGS) $^ -o $@

$(OUT_SERVER): $(OBJ_DIR)/server.o $(OBJ_DIR)/logger.o $(OBJ_DIR)/utils.o
	$(CC) $(CFLAGS) $^ -o $@



$(OBJ_DIR)/client.o: $(SRC_DIR)/client.c $(SRC_DIR)/logger.h $(SRC_DIR)/types.h $(SRC_DIR)/utils.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/server.o: $(SRC_DIR)/server.c $(SRC_DIR)/logger.h $(SRC_DIR)/types.h $(SRC_DIR)/utils.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/logger.o: $(SRC_DIR)/logger.c $(SRC_DIR)/logger.h $(SRC_DIR)/ansi_colors.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/utils.o: $(SRC_DIR)/utils.c $(SRC_DIR)/utils.h $(SRC_DIR)/ansi_colors.h $(SRC_DIR)/logger.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@



$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)



.PHONY: all clean

clean:
	rm -f $(OBJ_DIR)/*.o ./*.out
