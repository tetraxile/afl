TARGET_EXEC := main

BUILD_DIR := ./build
SRC_DIRS := ./src

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')
OBJS := $(SRCS:./%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := ./include
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS := $(INC_FLAGS) -MMD -MP
CXXFLAGS := -Werror -std=c++20 -O2
LDFLAGS :=

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean test
clean:
	rm -r $(BUILD_DIR)

TEST_OBJS := $(filter-out %main.cpp.o,$(OBJS)) $(BUILD_DIR)/perf/perf.cpp.o

test: $(OBJS)
	mkdir -p $(dir $(BUILD_DIR)/perf/perf.cpp)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c perf/perf.cpp -o $(BUILD_DIR)/perf/perf.cpp.o
	$(CXX) $(TEST_OBJS) -o $(BUILD_DIR)/test $(LDFLAGS)



-include $(DEPS)
