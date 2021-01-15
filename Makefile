# Makefile taken from https://spin.atomicobject.com/2016/08/26/makefile-c-projects/
# and added modifications to run with graph500

TARGET_EXEC ?= bfs

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src ./aml

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s) ./generator/graph_generator.c ./generator/make_graph.c ./generator/splittable_mrg.c ./generator/utils.c
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CFLAGS += -Drestrict=__restrict__ -O3 -DGRAPH_GENERATOR_MPI -DREUSE_CSR_FOR_VALIDATION -I../aml -g
CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -g
LDFLAGS += -lm -lpthread -lstdc++

# Compilers
CC = mpicc
CXX = mpic++

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

run:
	./build/$(TARGET_EXEC) 6

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
