NAME = webserv

CXX = c++
CXXFLAGS = -std=c++98 -g3

SRCS = 	main.cpp\
		cpp/Server.cpp\
		cpp/Client.cpp\
		cpp/Epoll.cpp\
		cpp/Epoll_utils.cpp\
		cpp/Location.cpp\
		cpp/Request.cpp\
		cpp/Response.cpp

OBJDIR = obj
OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re