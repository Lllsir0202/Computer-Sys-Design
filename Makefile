-include nemu/Makefile.git
# 获取当前分支名
CURRENT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)

# 定义目标分支
TARGET_BRANCH := origin/2017

default:
	@echo "Please run 'make' under any subprojects to compile."
 
clean:
	-$(MAKE) -C nemu clean
	-$(MAKE) -C nexus-am clean
	-$(MAKE) -C nanos-lite clean
	-$(MAKE) -C navy-apps clean

submit: clean
	git gc
	cd .. && tar cj $(shell basename `pwd`) > $(STU_ID).tar.bz2

count:
	@echo "当前分支: $(CURRENT_BRANCH)"
	@echo "比较 $(CURRENT_BRANCH) 与 $(TARGET_BRANCH) 的差异："
	@git diff $(TARGET_BRANCH)..$(CURRENT_BRANCH) --shortstat


.PHONY: default clean submit count
