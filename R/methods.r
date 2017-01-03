#' Plot and print methods for \code{phylo_offsprings} objects
#' @param x An object of class \code{phylo_offsprings}.
#' @param y Ignored.
#' @param layout Passed to \code{\link[igraph:plot.igraph]{igraph:plot.igraph}}
#' @param ... Further arguments passed to the method.
#' @rdname get_offsprings
#' @export
plot.phylo_offsprings <- function(
  x, y=NULL, layout=NULL, ...) {
  
  # Retrieving the edgelist
  edgelist   <- x[["edgelist"]]
  edgelist[] <- as.character(edgelist)
  
  # Converting into igraph object
  g <- igraph::graph_from_edgelist(edgelist)
  
  # Computing layout
  if (!length(layout)) {
    layout     <- igraph::layout_with_sugiyama(g, maxiter=200)[[2]]
    layout[,2] <- -layout[,2]
  }
  
  # Plotting
  plot(g, layout=layout,...)
  
}