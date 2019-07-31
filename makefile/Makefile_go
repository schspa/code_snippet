GOBUILD = go build
GOCLEAN = go clean
PFLAGS = -buildmode=plugin

all: main

main: main.go testplugin.so
	$(GOBUILD) -o $@ $(filter %.go, $^)

%.so: %.go
	$(GOBUILD) -o $@ $(PFLAGS) $^

clean:
	$(GOCLEAN) .

.PHONY: all clean
