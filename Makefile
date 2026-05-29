all:
	gcc -Wall -Wextra -o chatbot chatbot.c

clean:
	rm -f chatbot
