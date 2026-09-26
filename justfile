yk_root := "/path/to/yk"
yk_profile := "release"

build: build-plain

build-plain:
    rake -m -j$(nproc)

build-yk: build-plain
    YK_ROOT={{yk_root}} YK_PROFILE={{yk_profile}} rake -m -j$(nproc) MRUBY_CONFIG=yk_wordboxing

test: test-plain

test-plain: build-plain
    rake test

test-yk: build-yk
    build/yk_wordboxing/bin/mruby -e 'puts "hello"'

hello: build-plain
    build/host/bin/mruby -e 'puts "hello"'

hello-yk: build-yk
    build/yk_wordboxing/bin/mruby -e 'puts "hello"'

clean:
    rake clean
