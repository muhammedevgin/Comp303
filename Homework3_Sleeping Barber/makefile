
CC = gcc

CFLAGS = -g -Wall

TARGET = barber
SOURCE = barber

all: $(TARGET)
clean:
	rm -f $(TARGET)

$(TARGET): $(SOURCE).c
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE).c -lpthread

run: $(TARGET)
	./$(TARGET) $(num_customer) $(max_arrival_time) $(max_haircut_duration) $(haircut_repetition)

