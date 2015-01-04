TARGET = lensc
SRCS = $(shell find src -name "*.cpp")
OBJS = $(patsubst src/%.cpp,obj/%.o,$(SRCS))
DEPS = $(OBJS:%.o=%.d)
CFLAGS = -I./ --std=c++11
LIBS =

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(CFLAGS) $(LIBS)

obj/%.o: src/%.cpp
	$(CXX) -c $< -o $@ $(CFLAGS)

# Produce dependency files for objects
obj/%.d: src/%.cpp
	$(CXX) $(CFLAGS) -MM -MT '$(patsubst src/%.cpp,obj/%.o,$<)' $< -MF $@

# Ensure that the obj/ folder exists when it's needed
obj/:
	mkdir obj

# Include the generated dependencies
-include $(DEPS)