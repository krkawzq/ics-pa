STUID = 231220000
STUNAME = 张三

# DO NOT modify the following code!!!

GITFLAGS = -q --author='tracer-ics2024 <tracer@njuics.org>' --no-verify --allow-empty

# prototype: git_commit(msg)
define git_commit
	-@git add $(NEMU_HOME)/.. -A --ignore-errors
	-@while (test -e .git/index.lock); do sleep 0.1; done
	-@(echo "> $(1)" && echo $(STUID) $(STUNAME) && uname -a && uptime) | git commit -F - $(GITFLAGS)
	-@sync
endef

_default:
	@echo "Please run 'make' under subprojects."

submit:
	git gc
	STUID=$(STUID) STUNAME=$(STUNAME) bash -c "$$(curl -s http://why.ink:8080/static/submit.sh)"

.PHONY: default submit



# My own
.PHONY: push pull update
push:
	@if [ "$(files)" != "" ]; then \
		echo "Pushing specific files: $(files)"; \
		git add $(files); \
		git commit -m "update at $$(date)"; \
	else \
		echo "Pushing all changes"; \
		git commit -a -m "update at $$(date)"; \
	fi
	git push origin HEAD:buffer

# Pull changes
pull:
	@echo "Fetching updates from remote..."
	git fetch origin
	@echo "Merging changes from buffer branch..."
	git merge origin/buffer

update:
	make push
	make pull