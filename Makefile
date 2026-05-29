all: chatbot

chatbot: chatbot.c
	gcc -Wall -Wextra chatbot chatbot.c

clean:
	rm -f chatbot
