CC = gcc                    # Compiler to use
CFLAGS = -std=c99 -Wall -Wextra -O2  # Compiler flags: C99, warnings, optimizations
TARGET = syslang            # Name of the executable
OBJS = main.o syslang.o     # Object files to link

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

%.o: %.c syslang.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	del $(OBJS) $(TARGET) 2>nul || true

run: $(TARGET)
	./$(TARGET)