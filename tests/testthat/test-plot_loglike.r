context("plot_LogLike")

test_that("plotll", {
  set.seed(1)
  dat <- sim_annotated_tree(20)
  
  # aphylo method
  expect_silent(suppressWarnings(plot_logLik(dat)))

  # phylo_mle method
  ans <- suppressWarnings(aphylo_mle(dat ~ mu + psi))
  expect_silent(plot_logLik(ans))
  
})
