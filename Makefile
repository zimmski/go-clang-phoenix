.PHONY: all branch clean generate install install-dependencies install-tools lint test test-verbose

ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
export ROOT_DIR

ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
$(eval $(ARGS):;@:) # turn arguments into do-nothing targets
export ARGS

all: install-dependencies install-tools install test

branch:
	make switch-clang-version $(ARGS)
	scripts/branch.sh $(ARGS)
clean:
	rm -r clang-c/
	rm *_gen.go
install:
	CGO_CFLAGS="-I`llvm-config --includedir`" CGO_LDFLAGS="-L`llvm-config --libdir`" go install ./...
install-dependencies:
	go get -u golang.org/x/tools/imports/...
	go get -u github.com/stretchr/testify/...
	go get -u github.com/termie/go-shutil/...
install-tools:
	# Install linting tools
	go get -u golang.org/x/tools/cmd/vet/...
	go get -u github.com/golang/lint/...
	go get -u github.com/kisielk/errcheck/...

	# Install code coverage tools
	go get -u golang.org/x/tools/cmd/cover/...
	go get -u github.com/onsi/ginkgo/ginkgo/...
	go get -u github.com/modocache/gover/...
	go get -u github.com/mattn/goveralls/...
lint: install
	scripts/lint.sh
switch-clang-version:
	scripts/switch-clang-version.sh $(ARGS)
test:
	CGO_CFLAGS="-I`llvm-config --includedir`" CGO_LDFLAGS="-L`llvm-config --libdir`" go test -timeout 60s -race ./...
test-verbose:
	CGO_CFLAGS="-I`llvm-config --includedir`" CGO_LDFLAGS="-L`llvm-config --libdir`" go test -timeout 60s -race -v ./...
