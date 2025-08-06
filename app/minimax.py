import numpy as np
from numba import njit

SUN_SCORES_ARRAY = np.zeros(14, dtype=np.uint8)
SUN_SCORES_ARRAY[1]  = 11
SUN_SCORES_ARRAY[7]  = 0
SUN_SCORES_ARRAY[8]  = 0
SUN_SCORES_ARRAY[9]  = 0
SUN_SCORES_ARRAY[10] = 10
SUN_SCORES_ARRAY[11] = 2
SUN_SCORES_ARRAY[12] = 3
SUN_SCORES_ARRAY[13] = 4

SUN_RANK_ORDER = np.zeros(14, dtype=np.uint8)
SUN_RANK_ORDER[1] = 8
SUN_RANK_ORDER[7] = 1
SUN_RANK_ORDER[8] = 2
SUN_RANK_ORDER[9] = 3
SUN_RANK_ORDER[10] = 7
SUN_RANK_ORDER[11] = 4
SUN_RANK_ORDER[12] = 5
SUN_RANK_ORDER[13] = 6

@njit
def get_suit(card):
    return (card - 1) // 13
    
@njit
def get_rank(card):
    return (card - 1) % 13 + 1

@njit
def has_suit(cards, suit):
    for card in cards:
        if get_suit(card) == suit:
            return True
    return False

@njit
def get_trick_winner(trick, starter, SUN_RANK_ORDER):
    trick_suit = get_suit(trick[0])
    winner = starter
    max_value = SUN_RANK_ORDER[get_rank(trick[0])]

    for i in range(1, 4):
        card = trick[i]
        if get_suit(card) == trick_suit:
            value = SUN_RANK_ORDER[get_rank(card)]
            if value > max_value:
                max_value = value
                winner = (starter + i) % 4

    return winner

@njit
def numba_get_legal_moves(current_cards, played_cards, num_played, current_player, num_starting_cards):
    moves = np.empty(num_starting_cards, dtype=np.uint8)
    count = 0
    hand = current_cards[current_player]

    if num_played % 4 == 0:  # starting a new trick
        for card in hand:
            if card != 0:
                moves[count] = card
                count += 1
    else:
        first_card = played_cards[(num_played // 4) * 4]
        trick_suit = get_suit(first_card)

        has_trick_suit = False
        for card in hand:
            if card != 0 and get_suit(card) == trick_suit:
                has_trick_suit = True
                break

        for card in hand:
            if card == 0:
                continue
            if (has_trick_suit and get_suit(card) == trick_suit) or not has_trick_suit:
                moves[count] = card
                count += 1

    return moves[:count]

@njit
def numba_calculate_score(played_cards, num_played, sun_scores, sun_rank_order):
    if num_played % 4 != 0:
        return -1

    total = 0
    cur_starter = 0

    for i in range(0, num_played, 4):
        trick = played_cards[i:i+4]
        rel_winner = get_trick_winner(trick, 0, sun_rank_order)
        cur_starter = (cur_starter + rel_winner) % 4

        round_score = 0
        for j in range(4):
            rank = get_rank(played_cards[i + j])
            round_score += sun_scores[rank]

        if cur_starter == 0 or cur_starter == 2:
            total += round_score

        if i == num_played - 4 and (cur_starter == 0 or cur_starter == 2):
            total += 10

    return total

class GameState:
    def __init__(self, current_cards, NUM_OF_STARTING_CARDS):
        self.current_cards = current_cards
        self.played_cards = np.zeros(4 * NUM_OF_STARTING_CARDS, dtype='uint8')
        self.current_player = 0
        self.num_of_played_cards = 0
        self.card_indices = np.zeros(4 * NUM_OF_STARTING_CARDS, dtype='uint8') - 1
        self.player_indices = np.zeros(4 * NUM_OF_STARTING_CARDS, dtype='uint8') - 1
        self.NUM_OF_STARTING_CARDS = NUM_OF_STARTING_CARDS

    def get_legal_moves(self):
        return numba_get_legal_moves(self.current_cards, self.played_cards, self.num_of_played_cards, self.current_player, self.NUM_OF_STARTING_CARDS)

    def choose(self, card):
        if self.num_of_played_cards == self.NUM_OF_STARTING_CARDS * 4:
            raise ValueError("Cannot choose a card since all cards have been played.")
            

        current_player_cards = self.current_cards[self.current_player]
        card_idx = np.where(current_player_cards == card)[0][0]
        
        self.played_cards[self.num_of_played_cards] = card
        current_player_cards[card_idx] = 0

        self.player_indices[self.num_of_played_cards] = self.current_player
        self.card_indices[self.num_of_played_cards] = card_idx

        # UPDATE TO NEXT PLAYER
        if self.num_of_played_cards % 4 == 3:
            current_trick_starter = self.player_indices[self.num_of_played_cards - (self.num_of_played_cards % 4)]
            self.current_player = get_trick_winner(self.played_cards[self.num_of_played_cards - 3: self.num_of_played_cards + 1], current_trick_starter, SUN_RANK_ORDER)
        else:
            self.current_player = (self.current_player + 1) % 4
        
        self.num_of_played_cards += 1

    def unchoose(self):
        if self.num_of_played_cards == 0:
            raise ValueError("Cannot unchoose a card since no cards have been played.")

        self.current_player = self.player_indices[self.num_of_played_cards - 1]
        self.player_indices[self.num_of_played_cards - 1] = 255

        card_idx = self.card_indices[self.num_of_played_cards - 1]
        self.card_indices[self.num_of_played_cards - 1] = 255
        
        self.current_cards[self.current_player][card_idx] = self.played_cards[self.num_of_played_cards - 1]
        self.played_cards[self.num_of_played_cards - 1] = 0
            
        self.num_of_played_cards -= 1

    def calculate_score(self):
        return numba_calculate_score(self.played_cards, self.num_of_played_cards, SUN_SCORES_ARRAY, SUN_RANK_ORDER)
def game_state_minimax(game_state, depth, alpha, beta, maximizing, NUM_OF_STARTING_CARDS):
    global nodes_visited
    
    nodes_visited += 1
    
    if depth == 0 or game_state.num_of_played_cards == NUM_OF_STARTING_CARDS * 4:
        return game_state.calculate_score(), game_state.played_cards.copy()

    if maximizing:
        max_score = -float("inf")
        max_path = None
        for action in game_state.get_legal_moves():
            game_state.choose(action)
            score, path = game_state_minimax(game_state, depth - 1, alpha, beta, False if (game_state.current_player == 1 or game_state.current_player == 3) else True, NUM_OF_STARTING_CARDS)
            if score > max_score:
                max_score = score
                max_path = path
            game_state.unchoose()
            alpha = max(alpha, score)
            if beta <= alpha:
                break
        return max_score, max_path
    
    else:
        min_score = float("inf")
        min_path = None
        for action in game_state.get_legal_moves():
            game_state.choose(action)
            score, path = game_state_minimax(game_state, depth - 1, alpha, beta, False if (game_state.current_player == 1 or game_state.current_player == 3) else True, NUM_OF_STARTING_CARDS)
            if score < min_score:
                min_score = score
                min_path = path
            game_state.unchoose()
            beta = min(beta, score)
            if beta <= alpha:
                break
        return min_score, min_path

nodes_visited = 0