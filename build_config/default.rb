MRuby::Build.new do |conf|
  # load specific toolchain settings
  conf.toolchain

  # Matches ykmruby's build_config/default_yk_config.rb exactly (defines +
  # gembox set), so a plain build here stays comparable to the yk one -
  # same value representation, same gems, same call stack depth.
  conf.cc.defines << 'MRB_NO_DIRECT_THREADING'
  conf.cc.defines << 'MRB_WORD_BOXING'
  conf.cc.defines << 'MRB_INT64'
  # Default (512) is too low for AWFY's Havlak: HavlakLoopFinder#do_dfs
  # recurses once per CFG node on its fixed test graph, deep enough to
  # raise SystemStackError regardless of benchmark size. Matches
  # yk_wordboxing.rb's raised limit.
  conf.cc.defines << 'MRB_CALL_LEVEL_MAX=4096'

  conf.gembox 'stdlib'
  conf.gembox 'stdlib-ext'
  conf.gembox 'stdlib-io'
  conf.gembox 'math'
  conf.gembox 'metaprog'
  conf.gem core: 'mruby-bin-mruby'

  conf.enable_bintest
  conf.enable_test
end
