NAME = webserv

CXX = c++
CXXFLAGS = -std=c++98 -g3

SRCS = 	main.cpp\
		cpp/Server.cpp\
		cpp/Client.cpp\
		cpp/Epoll.cpp\
		cpp/Epoll_utils.cpp\
		cpp/Location.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re