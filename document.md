# UNIX26TASK3 Documentation

## Project Description
A C-based IRC chatbot that connects to the course IRC server, joins channels, and responds to taggged messages using a locally running LLM (Ollama).

## Implementation Details

### Process Structure
The program creates 3 processes:
1. **Parent**: Manages IRC socket connection, routes messages to children, handles PING/PONG
2. **Child 1**: Handles messages in #Unix2026 channel
3. **Child 2**: Handles messages in #viba1062 channel

### IPC Mechanisms Used
- **Pipes**: 4 pipe pairs for communication between parent and children
- **Signals**: SIGINT and SIGTERM
- **Shared Memory**: Created for IPC
- **Semaphores**: Initialized for synchronization

### Key Features
- Only responds to user `viba1062`
- Requires bot name `bviba1062` to be mentioned in message
- Never responds to other bots (users with 'b' prefix)
- Splits long responses to fit IRC message limits
- Logs all interactions to file

## How to Run the Project

### Step 1: Setup Ollama, install zstd and make, and pull tinyllama
Run the setup script:
chmod +x ollama-setup.sh
./ollama-setup.sh

### Step 2: Verify Ollama works
ollama run tinyllama "Hello, are you working?"

### Step 3: Compile the bot
cd ~/unix26task3
make

### Step 4: Run the bot
./chatbot
Keep this terminal open.

### Step 5: Connect to test
In a new terminal, connect to the IRC server:
irssi -c 10.0.1.172 -p 6667 -n viba1062
Then join the private channel:
/join #viba1062
Make sure to have irssi installed in that environment

### Step 6: Ask questions
Type your questions in the channel, tagging the bot:
bviba1062: What is a process in UNIX? Answer in 2-3 sentences
Wait for the response before asking the next question.
Its also not required to use bviba1062: <question> syntax, you can also write something like "hello bviba1062 <question>.

### Step 7: Stop the bot
Press Ctrl+C in the bot terminal to stop it.

### Step 8: Check logs
All bot interactions are saved to:
cat ~/unix26task3/bot_interactions.log


## Challenges Encountered

### Connection Issues
The bot initially tried to JOIN channels before full registration with the IRC server was complete. The server returned "You must be fully connected to use this command." Fixed by waiting for the 001 welcome message before sending JOIN commands.

### Ollama Output
Ollama outputs ANSI escape codes and progress bars that appeared as garbage characters in IRC messages. Used sed and tr to pipe the output through cleanup filters

### IRC Message Limits
Responses longer than 512 characters were being truncated by the IRC server. Implemented chunk splitting to break responses into 400-character pieces and send them as separate messages with a small delay between each.

### VM Performance
The VM has no GPU and limited CPU, causing Ollama responses to take a long time.

### Incomplete Responses
Tinyllama being a small model sometimes cuts off mid-sentence.

## Test Questions and Responses

### Question 1
viba1062: bviba1062: What is Unix?
bviba1062:Unix is an operating system (OS) designed to provide a common environment f for computer users,
and it has become the standard operating system (S/O S) S) for many modern computers. Unix differs from other
operating systems suc such as Windows or Mac OS in several ways:
1. It is free and open-source software that is distributed under the GNU Ge General Public License (GPL). This means that anyone can download, modify,
 and distribute the source code for the operating system without requiring p permission.

### Question 2
viba1062: bviba1062: What is a fork?
bviba1062: A fork is a type of branching operation in programming that creates a new b branch off the main (or ancestral) line of code.
The fork operation creates creates a copy of the current file, which is usually named after the forked forked file or directory name to 
differentiate it from its parent directory directory. This new file will be treated as an independent branch and can b be edited, deleted o
r moved separately from the main (or ancestral) branch. branch. A fork does not modify the original file or dir

### Question 3
viba1062: bviba1062: What is a pipe in Unix? Answer in 2-3 sentences
bviba1062: In UNIX, a "pipe" is a communication channel that allows fo for the sending and receiving of data between processes.
It consists of two two ends called "in" (input) and "out" (output), with data flowing from one one to the other.
The name "pipe" comes from "pipe-shaped", referring to it its shape, which is similar to a fishbowl.

### Question 4
viba1062: hello bviba1062 what is the difference between a process and a thread?
bviba1062: A process and a thread are two different concepts in computer science. Here Here's a brief explanation of each:
- Process: A process is a group of tasks or jobs that execute independently independently of other processes running on a computer.
-  It has its own memo memory, CPU, and resources, such as files, data, and network connections. A A process is responsible for managing its own resources and
PRIVMSG #viba1062 : coordinating wi with other processes to complete its tasks. - Thread: A thread is a set of  operations or inst

### Question 5
viba1062:bviba1062 What are file permissions in UNIX? Answer in 2 sentences
bviba1062:  File permissions in UNIX refer to the access rights granted by the operatin operating system to
                   files and directories. The permissions provide control  over who can read, write, execute or delete
                   a particular file or directory, directory, and allow users with higher-level privileges (such as
                   root) to p perform actions on lower-level privileges (such as regular users).  In 2 sentences:  1.
                   UNIX fil
e permissions determine who has access to what files and directo directories, and which actions they
                   can perfor

## Files
- chatbot.c / chatbot.h
- irc.c / irc.h 
- ollama.c / ollama.h
- Makefile
- ollama-setup.sh
- bot_interactions.log (created when running the project)
