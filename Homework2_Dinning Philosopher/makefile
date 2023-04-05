CC = gcc

CFLAGS = -g -Wall

TARGET = phsp
SOURCE = phsp

all: $(TARGET)

$(TARGET): $(SOURCE).c
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE).c -lpthread -lm

run: $(TARGET)
	./$(TARGET) $(num_phsp) $(min_think) $(max_think) $(min_dine) $(max_dine) $(dst) $(num_dine)

clean:
	rm -f $(TARGET)