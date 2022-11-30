#include "query_exec.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "dataForm.h"
#include "map_info.h"
#include "partitioner.h"
#include "simple_vector.h"

//-----------------------------------------------------------------------------------------

void QueryExec::execute(char* query) {
  parse_query(query);
  do_query();

  // Clear all simple_vectors to prepare for next Query
  clear();
}

//-----------------------------------------------------------------------------------------

void QueryExec::parse_query(char* query) {
  char* buffr;

  char* used_relations = strtok_r(query, "|", &buffr);
  parse_names(used_relations);

  // for (size_t i = 0; i < rel_names.getSize(); i++) {
  //   std::printf("rel %ld\n", rel_names[i]);
  // }

  char* predicates = strtok_r(nullptr, "|", &buffr);
  parse_predicates(predicates);

  // for (size_t i = 0; i < joins.getSize(); i++) {
  //   std::fprintf(stderr, "%ld.%ld %c %ld.%ld\n", joins[i].left_rel,
  //                joins[i].left_col,
  //                (joins[i].op == 0) ? ('=') : ((joins[i].op == 1) ? '>' :
  //                '<'), joins[i].right_rel, joins[i].right_col);
  // }
  // for (size_t i = 0; i < filters.getSize(); i++) {
  //   std::fprintf(
  //       stderr, "%ld.%ld %c %ld\n", filters[i].left_rel, filters[i].left_col,
  //       (filters[i].op == 0) ? ('=') : ((filters[i].op == 1) ? '>' : '<'),
  //       filters[i].literal);
  // }

  // buffr now points to the last part of the query
  char* selections = buffr;
  parse_selections(selections);

  //   for (size_t i = 0; i < projections.getSize(); i++) {
  //     std::printf("projection rel %ld col %ld\n", projections[i].rel,
  //                 projections[i].col);
  //   }
}

void QueryExec::parse_names(char* rel_string) {
  char *buffr, *ignore;
  char* rel;

  while ((rel = strtok_r(rel_string, " ", &buffr))) {
    this->rel_names.add_back(std::strtol(rel, &ignore, 10));
    rel_string = nullptr;
  }
}

void QueryExec::parse_predicates(char* predicates) {
  char* ignore;
  char *buffr, *buffr2, *buffr3;
  const char* op_val;

  char* predicate;

  operators operation_type;

  while ((predicate = strtok_r(predicates, "&", &buffr))) {
    if (std::strchr(predicate, '=')) {
      operation_type = operators::EQ;
      op_val = "=";
    } else if (std::strchr(predicate, '>')) {
      operation_type = operators::GREATER;
      op_val = ">";
    } else {
      operation_type = operators::LESS;
      op_val = "<";
    }

    char* left = strtok_r(predicate, op_val, &buffr2);

    if (std::strchr(left, '.') == nullptr) {
      // left contains literal - 100% filter
      long literal = std::strtol(left, &ignore, 10);

      if (operation_type == operators::GREATER)
        operation_type = operators::LESS;
      else if (operation_type == operators::LESS)
        operation_type = operators::GREATER;
      else
        operation_type = operators::EQ;

      char* right = buffr2;

      long right_rel = std::strtol(strtok_r(right, ".", &buffr3), &ignore, 10);

      long right_col = std::strtol(buffr3, &ignore, 10);

      filter myfilter(right_rel, right_col, operation_type, literal);
      this->filters.add_back(myfilter);

    } else {
      // e.g 0.2 (relation.column) - Could be filter OR join
      long left_rel = std::strtol(strtok_r(left, ".", &buffr3), &ignore, 10);
      long left_col = std::strtol(buffr3, &ignore, 10);

      char* right = buffr2;
      if (std::strchr(right, '.') == nullptr) {
        // right contains literal - 100% filter
        long literal = std::strtol(right, &ignore, 10);

        filter myfilter(left_rel, left_col, operation_type, literal);
        this->filters.add_back(myfilter);
      } else {
        // e.g 1.4 (relation.column) - 100% join (but can be same relation!!!)
        long right_rel =
            std::strtol(strtok_r(right, ".", &buffr3), &ignore, 10);
        long right_col = std::strtol(buffr3, &ignore, 10);

        join myjoin(left_rel, left_col, operation_type, right_rel, right_col);
        this->joins.add_back(myjoin);
      }
    }
    predicates = nullptr;
  }
}

void QueryExec::parse_selections(char* selections) {
  char *buffr, *buffr2;
  char* ignore;
  char* selection;

  while ((selection = strtok_r(selections, " ", &buffr))) {
    char* rel = strtok_r(selection, ".", &buffr2);
    char* col = buffr2;

    this->projections.add_back(project_rel{std::strtol(rel, &ignore, 10),
                                           std::strtol(col, &ignore, 10)});

    selections = nullptr;
  }
}

//-----------------------------------------------------------------------------------------

void QueryExec::do_query() {
  intmd_count = 0;
  intmd = new simple_vector<
      int64_t>[this->rel_names.getSize()];  // Represents latest
                                            // intermediate results

  goes_with = new simple_vector<int64_t>[this->rel_names.getSize()];

  // intmd_results = new simple_vector<simple_vector>[this->filters.getSize() +
  //                                                 this->joins.getSize()];
  //   simple_vector<int64_t>* intmd_results =
  //       new simple_vector<int64_t>[this->rel_names.getSize()];

  // set all relations to not have been used initially
  for (size_t i = 0; i < this->rel_names.getSize(); i++)
    this->used_relations.add_back(false);

  // Check whether there are filters in order to execute them first
  for (size_t i = 0; i < this->filters.getSize(); i++) {
    filter_exec(i);
    intmd_count++;
    // std::fprintf(stderr, "filter %ld done\n", i);
  }

  for (size_t i = 0; i < this->joins.getSize(); i++) {
    // std::fprintf(stderr, "%ld.%ld=%ld.%ld\n", this->joins[i].left_rel,
    //              this->joins[i].left_col, this->joins[i].right_rel,
    //              this->joins[i].right_col);
    do_join(i);
    intmd_count++;
    // std::fprintf(stderr, "join %ld done\n", i);
  }

  //   for (size_t i = 0; i < this->joins.getSize(); i++) {
  //     // Check for Self-joins
  //     if (this->rel_names[joins[i].left_rel] ==
  //         this->rel_names[joins[i].right_rel]) {
  //       // Self-join found - Execute it
  //       do_self_join(i);
  //       intmd_count++;
  //     }
  //     // Rest stuff
  //     else {
  //       if (used_relations[joins[i].left_rel] == true &&
  //           used_relations[joins[i].right_rel] == true) {
  //         // Both relations exist in intermediate results
  //         // do_simple_join(intmd_results[joins[i].left_rel],
  //         //               intmd_results[joins[i].right_rel], i);
  //       } else {
  //         // Execute Partitioned Hash Join (PHJ)
  //         // do_hash_join(intmd_results[joins[i].left_rel],
  //         //             intmd_results[joins[i].right_rel], i);
  //       }
  //     }
  //   }

  // Check for Self-joins
  //   for (size_t i = 0; i < this->joins.getSize(); i++) {
  //     if (joins[i].left_rel == joins[i].right_rel) {
  //       // Self-join found - Execute it
  //       do_self_join(i);
  //     }
  //   }

  // Execute the rest -
  // for (size_t i = 0; i < this->joins.getSize(); i++) {
  //   if (joins[i].left_rel != joins[i].right_rel) {
  //     // Both relations exist in intermediate results
  //     // Execute Simple-Join
  //     if (used_relations[joins[i].left_rel] == true &&
  //         used_relations[joins[i].right_rel] == true) {
  //       do_simple_join(intmd_results[joins[i].left_rel],
  //                      intmd_results[joins[i].right_rel], i);
  //       // Execute Partitioned Hash Join (PHJ)
  //     } else if (used_relations[joins[i].left_rel] == false ||
  //                false == used_relations[joins[i].right_rel]) {
  //       do_hash_join(intmd_results[joins[i].left_rel],
  //                    intmd_results[joins[i].right_rel], i);
  //     }
  //   }
  // }

  // Done with all predicates
  // Execute Checksum on given projections
  checksum();

  // Clear intmd & goes_with simple_vectors to prepare for next query
  for (size_t i = 0; i < this->rel_names.getSize(); i++) {
    intmd[i].clear();
    goes_with[i].clear();
  }
}

//-----------------------------------------------------------------------------------------

void QueryExec::filter_exec(size_t index) {
  int64_t rel = this->filters[index].rel;
  int64_t actual_rel = this->rel_names[rel];

  int64_t col = this->filters[index].col;
  int64_t lit = this->filters[index].literal;
  operators operation_type = this->filters[index].op;
  // std::fprintf(stderr, "%d\n", operation_type);

  simple_vector<int64_t>* new_intmd =
      new simple_vector<int64_t>[this->rel_names.getSize()];
  ;

  // Relation hasn't been used in a predicate before
  if (used_relations[rel] == false) {
    // First time applying a predicate on this relation
    // Mark the relation as used
    used_relations[rel] = true;
    if (intmd_count != 0)
      // We have intermediate results from previous predicates
      new_intmd = intmd;
    // We have to traverse the initial relation
    // and fill only the simple_vector for given relation
    for (uint64_t row = 0; row < rel_mmap[actual_rel].rows; row++) {
      switch (operation_type) {
        case operators::EQ:
          if ((int64_t)rel_mmap[actual_rel].colptr[col][row] == lit)
            new_intmd[rel].add_back(row);
          break;
        case operators::GREATER:
          if ((int64_t)rel_mmap[actual_rel].colptr[col][row] > lit)
            new_intmd[rel].add_back(row);
          break;
        case operators::LESS:
          if ((int64_t)rel_mmap[actual_rel].colptr[col][row] < lit)
            new_intmd[rel].add_back(row);
          break;
        default:
          std::perror("Unknown operator\n");
          exit(EXIT_FAILURE);
      }
    }
  } else {
    // Relation has already been used in a predicate before
    // We have to traverse the intermediate results of the
    // corresponding relation
    if (intmd_count == 0) {
      std::perror("No intermediate results found\n");
      exit(EXIT_FAILURE);
    } else {
      // We have intermediate results from previous predicates
      for (size_t x = 0; x < this->rel_names.getSize(); x++) {
        if ((int64_t)x != rel)
          new_intmd[x] = intmd[x];
        else {
          for (size_t i = 0; i < intmd[rel].getSize(); i++) {
            int64_t curr_row = intmd[rel][i];

            switch (operation_type) {
              case operators::EQ:
                if ((int64_t)rel_mmap[actual_rel].colptr[col][curr_row] == lit)
                  new_intmd[rel].add_back(curr_row);
                break;
              case operators::GREATER:
                if ((int64_t)rel_mmap[actual_rel].colptr[col][curr_row] > lit)
                  new_intmd[rel].add_back(curr_row);
                break;
              case operators::LESS:
                if ((int64_t)rel_mmap[actual_rel].colptr[col][curr_row] < lit)
                  new_intmd[rel].add_back(curr_row);
                break;
              default:
                std::perror("Unknown operator\n");
                exit(EXIT_FAILURE);
            }
          }
        }
      }
    }
  }
  intmd = new_intmd;
}

void QueryExec::do_join(size_t join_index) {
  int64_t rel_r = joins[join_index].left_rel;
  int64_t col_r = joins[join_index].left_col;
  int64_t rel_s = joins[join_index].right_rel;
  int64_t col_s = joins[join_index].right_col;
  int64_t actual_rel_r = this->rel_names[rel_r];
  int64_t actual_rel_s = this->rel_names[rel_s];

  simple_vector<int64_t>* new_intmd =
      new simple_vector<int64_t>[this->rel_names.getSize()];
  ;

  // Relation r hasn't been used in a predicate before
  if (used_relations[rel_r] == false) {
    // First time applying a predicate on relation r
    // Mark relation r as used
    used_relations[rel_r] = true;
    if (intmd_count != 0) {
      // We have intermediate results from previous predicates
      // but do not belong to relation r
      // Relation s hasn't been used in a predicate before
      if (used_relations[rel_s] == false) {
        // Mark the relation as used
        used_relations[rel_s] = true;

        // Take those intermediate results
        for (size_t i = 0; i < this->rel_names.getSize(); i++) {
          if ((int64_t)i != rel_r && (int64_t)i != rel_s)
            new_intmd[i] = intmd[i];
        }

        // We need to join those two
        for (size_t row = 0; row < rel_mmap[actual_rel_r].rows; row++) {
          for (size_t i = 0; i < rel_mmap[actual_rel_s].rows; i++) {
            if (rel_mmap[actual_rel_r].colptr[col_r][row] ==
                rel_mmap[actual_rel_s].colptr[col_s][i]) {
              new_intmd[rel_r].add_back(row);
              new_intmd[rel_s].add_back(i);
            }
          }
        }
        goes_with[rel_r].add_back(rel_s);
        goes_with[rel_s].add_back(rel_r);
      } else {
        // Relation s has intermediate results
        for (size_t row = 0; row < intmd[rel_s].getSize(); row++) {
          for (size_t i = 0; i < rel_mmap[actual_rel_r].rows; i++) {
            if (rel_mmap[actual_rel_s].colptr[col_s][row] ==
                rel_mmap[actual_rel_r].colptr[col_r][i]) {
              new_intmd[rel_r].add_back(row);
              new_intmd[rel_s].add_back(i);

              for (size_t x = 0; x < goes_with[rel_s].getSize(); x++) {
                int64_t related = goes_with[rel_s][x];
                new_intmd[related].add_back(intmd[related][row]);
              }
            }
          }
        }
        for (size_t x = 0; x < this->rel_names.getSize(); x++) {
          if (goes_with[rel_s].find(x)) continue;
          new_intmd[x] = intmd[x];
        }

        goes_with[rel_r].add_back(rel_s);
        goes_with[rel_s].add_back(rel_r);
      }
    } else {
      // No intermediate results
      used_relations[rel_s] = true;
      // We need to join those two
      for (size_t row = 0; row < rel_mmap[actual_rel_r].rows; row++) {
        for (size_t i = 0; i < rel_mmap[actual_rel_s].rows; i++) {
          if (rel_mmap[actual_rel_r].colptr[col_r][row] ==
              rel_mmap[actual_rel_s].colptr[col_s][i]) {
            new_intmd[rel_r].add_back(row);
            new_intmd[rel_s].add_back(i);
          }
        }
      }
      goes_with[rel_r].add_back(rel_s);
      goes_with[rel_s].add_back(rel_r);
    }
  } else {
    // Relation r has already been used in a predicate before
    // We have to traverse the intermediate results of the
    // corresponding relation
    if (intmd_count == 0) {
      std::perror("No intermediate results found\n");
      exit(EXIT_FAILURE);
    } else {
      if (used_relations[rel_s] == false) {
        used_relations[rel_s] = true;

        for (size_t row = 0; row < intmd[rel_r].getSize(); row++) {
          for (size_t i = 0; i < rel_mmap[actual_rel_s].rows; i++) {
            if (rel_mmap[actual_rel_r].colptr[col_r][row] ==
                rel_mmap[actual_rel_s].colptr[col_s][i]) {
              new_intmd[rel_r].add_back(row);
              new_intmd[rel_s].add_back(i);

              for (size_t x = 0; x < goes_with[rel_r].getSize(); x++) {
                int64_t related = goes_with[rel_r][x];
                new_intmd[related].add_back(intmd[related][row]);
              }
            }
          }
        }
        for (size_t x = 0; x < this->rel_names.getSize(); x++) {
          if (goes_with[rel_r].find(x)) continue;
          new_intmd[x] = intmd[x];
        }

        goes_with[rel_r].add_back(rel_s);
        goes_with[rel_s].add_back(rel_r);
      } else {
        for (size_t row = 0; row < intmd[rel_r].getSize(); row++) {
          for (size_t i = 0; i < intmd[rel_s].getSize(); i++) {
            if (rel_mmap[actual_rel_r].colptr[col_r][row] ==
                rel_mmap[actual_rel_s].colptr[col_s][i]) {
              new_intmd[rel_r].add_back(row);
              new_intmd[rel_s].add_back(i);

              for (size_t x = 0; x < goes_with[rel_r].getSize(); x++) {
                int64_t related = goes_with[rel_r][x];
                if (related == rel_s) break;
                new_intmd[related].add_back(intmd[related][row]);
              }

              for (size_t x = 0; x < goes_with[rel_s].getSize(); x++) {
                int64_t related = goes_with[rel_s][x];
                if (related == rel_r) break;
                new_intmd[related].add_back(intmd[related][row]);
              }
            }
          }
        }
        if (!(goes_with[rel_r].find(rel_s))) goes_with[rel_r].add_back(rel_s);
        if (!(goes_with[rel_s].find(rel_r))) goes_with[rel_s].add_back(rel_r);
      }
    }
  }
  intmd = new_intmd;
}

// simple_vector<result_item> QueryExec::do_simple_join(
//     simple_vector<int64_t>& rowids_r, simple_vector<int64_t>& rowids_s,
//     int64_t simplejoin_index) {
//   int64_t rel_r = joins[simplejoin_index].left_rel;
//   int64_t col_r = joins[simplejoin_index].left_col;

//   int64_t rel_s = joins[simplejoin_index].right_rel;
//   int64_t col_s = joins[simplejoin_index].right_col;

//   simple_vector<result_item> next;

//   // If-Statement not really needed, just to make sure
//   if (used_relations[rel_r] == true && used_relations[rel_s] == true) {
//     const size_t row_count_r = rowids_r.getSize();
//     const size_t row_count_s = rowids_s.getSize();
//     for (size_t i = 0; i < row_count_r; i++) {
//       int64_t rowid_r = rowids_r[i];
//       for (size_t j = 0; j < row_count_s; j++) {
//         int64_t rowid_s = rowids_s[j];
//         if (rel_mmap[rel_r].colptr[col_r][rowid_r] ==
//             rel_mmap[rel_s].colptr[col_s][rowid_s])
//           next.add_back(result_item{rowid_r, rowid_s});
//       }
//     }
//   } else {
//     std::perror("do_simple_join failed");
//     exit(EXIT_FAILURE);
//   }

//   return next;
// }

// result QueryExec::do_hash_join(simple_vector<int64_t>& rowids_r,
//                                simple_vector<int64_t>& rowids_s,
//                                int64_t index_join) {
//   size_t tuples_count_r = rowids_r.getSize();
//   size_t tuples_count_s = rowids_s.getSize();

//   tuple* rtuples = new tuple[tuples_count_r];
//   tuple* stuples = new tuple[tuples_count_s];

//   int64_t relation_r = joins[index_join].left_rel;
//   int64_t relation_s = joins[index_join].right_rel;

//   used_relations[relation_r] = used_relations[relation_s] = true;

//   int64_t join_colr = joins[index_join].left_col;
//   int64_t join_cols = joins[index_join].right_col;

//   uint64_t* relation_colr = rel_mmap[relation_r].colptr[join_colr];
//   uint64_t* relation_cols = rel_mmap[relation_s].colptr[join_cols];

//   for (size_t i = 0; i < tuples_count_r; i++) {
//     int64_t rowid = rowids_r[i];
//     int64_t val = relation_colr[rowid];
//     rtuples[i] = {val, rowid};
//   }

//   for (size_t i = 0; i < tuples_count_s; i++) {
//     int64_t rowid = rowids_s[i];
//     int64_t val = relation_cols[rowid];
//     stuples[i] = {val, rowid};
//   }

//   relation r(rtuples, tuples_count_r);
//   relation s(stuples, tuples_count_s);

//   return PartitionedHashJoin(r, s);
// }

void QueryExec::checksum() {
  int64_t curr_rel;
  int64_t curr_col;
  int64_t curr_row;
  int64_t sum;
  int64_t actual_rel;

  for (size_t i = 0; i < this->projections.getSize(); i++) {
    curr_rel = this->projections[i].rel;
    curr_col = this->projections[i].col;
    actual_rel = this->rel_names[curr_rel];
    sum = 0;

    // std::fprintf(stderr, "Size = %ld\n", intmd[curr_rel].getSize());

    for (size_t j = 0; j < intmd[curr_rel].getSize(); j++) {
      curr_row = intmd[curr_rel][j];

      sum += rel_mmap[actual_rel].colptr[curr_col][curr_row];
    }

    if (sum == 0) std::fprintf(stderr, "NULL ");
    // std::printf("NULL ");
    else
      std::fprintf(stderr, "%ld ", sum);
    // std::printf("%ld ", sum);
  }
  std::fprintf(stderr, "\n");
  // std::printf("\n");
}

//-----------------------------------------------------------------------------------------

void QueryExec::clear() {
  this->rel_names.clear();
  this->joins.clear();
  this->filters.clear();
  this->projections.clear();
  this->used_relations.clear();
  delete[] this->intmd;
  delete[] this->goes_with;
}