yk_config := "/path/to/yk-config"
yk_profile := "release"

build: build-plain

build-plain:
    rake -m -j$(nproc)

build-yk: build-plain
    YK_CONFIG={{yk_config}} YK_PROFILE={{yk_profile}} rake clean MRUBY_CONFIG=yk_build_config
    YK_CONFIG={{yk_config}} YK_PROFILE={{yk_profile}} rake -m -j$(nproc) MRUBY_CONFIG=yk_build_config

test: test-plain

test-plain: build-plain
    rake test

test-yk: build-yk
    build/yk_build_config/bin/mruby -e 'puts "hello"'

hello: build-plain
    build/host/bin/mruby -e 'puts "hello"'

hello-yk: build-yk
    build/yk_build_config/bin/mruby -e 'puts "hello"'

clean:
    rake clean
