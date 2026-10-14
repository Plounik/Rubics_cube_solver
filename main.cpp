#include <iostream>
using namespace std;
#include <bitset>
#include <string>
#include <chrono>
#include <vector>


uint64_t SOLVED_CORNER = 0b00111'00110'00101'00100'00011'00010'00001'00000;
uint64_t SOLVED_EDGE = 0b01011'01010'01001'01000'00111'00110'00101'00100'00011'00010'00001'00000;

uint64_t corner_position = SOLVED_CORNER;
uint64_t edge_position = SOLVED_EDGE;


char face[6] = {'U', 'F', 'R', 'D', 'B', 'L'};

int U[3][8][2]; int F[3][8][2]; int R[3][8][2]; int D[3][8][2]; int B[3][8][2]; int L[3][8][2];

int (*face1[6])[8][2] = {U, F, R, D, B, L};
uint64_t masks[6][2];
uint64_t c_masks[6][2];

uint64_t turn_piece(uint64_t pos, int p0[2], int p1[2], int p2[2], int p3[2], uint64_t mask);
void turn(int move, int index, uint64_t* corner, uint64_t* edge);
void turn_string(string move);
void tables();
void go_depth(int depth, int max_depth, uint64_t corner_search_pos, uint64_t edge_search_pos, int start);
void search(int max_depth, uint64_t corner_search_pos, uint64_t edge_search_pos);
void check_position(uint64_t corner_pos, uint64_t edge_pos, int move_num);


int counter = 0;
int max_solved_pieces = 0;
bool solved = false;
vector<int> path;
vector<int> stored_path;


int main(){
  tables();
  auto start = chrono::steady_clock::now();

  search(6, corner_position, edge_position);

  

  auto end = chrono::steady_clock::now();

  chrono::duration<double> elapsed = end - start;
  cout << "Time: " << elapsed.count() << " seconds\n";


  cout << counter;
  return 0;
}





void go_depth(int depth, int max_depth, uint64_t corner_search_pos, uint64_t edge_search_pos, int start){
  uint64_t corner_search_pos_copy = corner_search_pos;
  uint64_t edge_search_pos_copy = edge_search_pos;
  check_position(corner_search_pos_copy, edge_search_pos_copy, start + depth);
  if(depth < max_depth){
    for(int move = 0; move < 6; move++){
      for(int index = 0; index < 3; index++){
        if(!solved){
          counter += 1;
          path[start + depth] = move * 3 + index;
          corner_search_pos_copy = corner_search_pos;
          edge_search_pos_copy = edge_search_pos;
          turn(move, index, &corner_search_pos_copy, &edge_search_pos_copy);
          go_depth(depth+1, max_depth, corner_search_pos_copy, edge_search_pos_copy, start);
      }
      }
    }
  }
}

void search(int max_depth, uint64_t corner_search_pos, uint64_t edge_search_pos){
  path = stored_path;
  int start = path.size();
  path.resize(start + max_depth);
  go_depth(0, max_depth, corner_search_pos, edge_search_pos, start);
}



void check_position(uint64_t corner_pos, uint64_t edge_pos, int move_num){
  uint64_t c_mask = 0b00001'00001'00001'00001'00001'00001'00001'00001;
  uint64_t e_mask = 0b00001'00001'00001'00001'00001'00001'00001'00001'00001'00001'00001'00001;
  uint64_t c_difference = corner_pos ^ SOLVED_CORNER;
  uint64_t e_difference = edge_pos ^ SOLVED_EDGE;
  uint64_t c_combined =  c_difference | (c_difference >> 1) | (c_difference >> 2) | (c_difference >> 3) | (c_difference >> 4);
  uint64_t e_combined =  e_difference | (e_difference >> 1) | (e_difference >> 2) | (e_difference >> 3) | (e_difference >> 4);
  int solved_pieces = __popcnt64((~c_combined) & c_mask) + __popcnt64((~e_combined) & e_mask);
  if(solved_pieces > max_solved_pieces){
    max_solved_pieces = solved_pieces;
    stored_path.assign(
    path.begin(),
    path.begin() + move_num
);
  }
  if(solved_pieces == 20){
    cout << "cube solved\n";
    solved = true;
  }
  counter += 1;
}




uint64_t turn_piece(uint64_t pos, int p0[2], int p1[2], int p2[2], int p3[2], uint64_t mask){
  uint8_t m = 31;
  int* P[4] = {p0, p1, p2, p3};
  uint64_t V[4];
  for (int i = 0; i < 4; ++i){
    V[i] = ((pos >> (P[i])[0]) & m) << P[i][1];
  }
  return((pos & mask) | V[0] | V[1] | V[2] | V[3]);
}

void turn(int move, int index, uint64_t* corner, uint64_t* edge){  
  uint64_t mask_F = 0b000000000000000100000000000000100001000000000000000000010000; // move 0 = U, 1 = F, 2 = R, 3 = D, 4 = B, 5 = L
  uint64_t mask_B = 0b000001000000000000001000010000000000000000000100000000000000; // index 0 = counterclockwise, 1 = clockwise, 2 = 180
   auto [c0, c1, c2, c3, e0, e1, e2, e3] = face1[move][index];     
  *corner = turn_piece(*corner, c0, c1, c2, c3, masks[move][0]);
  *edge = turn_piece(*edge, e0, e1, e2, e3, masks[move][1]);

  if (index != 2) {
    if (move == 1) *edge ^= mask_F;
    else if (move == 4) *edge ^= mask_B;
    uint64_t low_mask  = c_masks[move][0];
    uint64_t high_mask = c_masks[move][1];
    switch (move % 3) {
      case 0: { 
        uint64_t low_bits  = *corner & low_mask;
        uint64_t high_bits = *corner & high_mask;

        *corner =
            (*corner & ~(low_mask | high_mask))
            | (low_bits << 1)
            | (high_bits >> 1);
        break;
      }

      case 1: 
        *corner ^= (~*corner & low_mask) << 1;
        break;

      case 2: 
        *corner ^= (~*corner & high_mask) >> 1;
        break;
    }     
  } 
}

void turn_string(string move){
  int m;
  int index;
  switch(move[0]){
    case 'U':
      m = 0;
      break;
    case 'F':
      m = 1;
      break;
    case 'R':
      m = 2;
      break;
    case 'D':
      m = 3;
      break;
    case 'B':
      m = 4;
      break;
    case 'L':
      m = 5;
      break;
  }

  switch (move[1]){
    case '2':
      index = 2;
      break;
    case '\'':
      index = 0;
      break;
    default:
      index = 1;
      break;
  }
  turn(m, index, &corner_position, &edge_position);
}

void tables(){
  int faces[6][8] = {
  {0, 1, 2, 3, 0, 1, 2, 3},
  {0, 4, 5, 1, 0, 4, 8, 5},
  {0, 3, 7, 4, 3, 7, 11, 4},
  {4, 7, 6, 5, 8, 11, 10, 9},
  {3, 2, 6, 7, 2, 6, 10, 7},
  {1, 5, 6, 2, 1, 5, 9, 6}
  };
  for (int i = 0; i < 6; ++i){
    auto [c0, c1, c2, c3, e0, e1, e2, e3] = faces[i];

    int d = -1;

    int c[4] = {c0, c1, c2, c3};
    int e[4] = {e0, e1, e2, e3};

    uint64_t c_mask;
    for (int j = 0; j < 3; ++j){
      c_mask = 0;
        for (int k = 0; k < 4; ++k){
          int next = (k + 2 - d) % 4;
          face1[i][j][k][0] = c[k] * 5;
          face1[i][j][k][1] = c[next] * 5;

          face1[i][j][k + 4][0] = e[k] * 5;
          face1[i][j][k + 4][1] = e[next] * 5;
          c_mask |= uint64_t{1} << (5 * c[k] + 3);
        }
        d += 2;
        d %= 3;
      }
      c_masks[i][0] = c_mask;
      c_masks[i][1] = c_mask << 1;
    uint64_t mask_c = (uint64_t{1} << 40) - 1;
    uint64_t mask_e = (uint64_t{1} << 60) - 1;
    for (int k = 0; k < 4; ++k) {
      mask_c &= ~(uint64_t{31} << (c[k] * 5));
      mask_e &= ~(uint64_t{31} << (e[k] * 5));
    }
    masks[i][0] = mask_c;
    masks[i][1] = mask_e;
    }
  }
