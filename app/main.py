import streamlit as st
import numpy as np
import time
from minimax import GameState, game_state_minimax, get_suit, get_rank, nodes_visited

SUITS = ["♥", "♠", "♦", "♣"]
RANKS = ["A", "10", "K", "Q", "J", "9", "8", "7"]

OPTIONS = [f"{rank}{suit}" for rank in RANKS for suit in SUITS]

st.title("Baloot AI")

st.markdown("""
**Baloot** is a popular and strategic 4-player card game widely played in Saudi Arabia, especially in social settings and friendly competitions. Played in teams of two, it emphasizes coordination, memory, and decision-making, making it both engaging and intellectually challenging.

---

### Game Overview

- **Teams:**  
  Two teams compete — **Player 1 & Player 3** vs **Player 2 & Player 4**.
  
- **Turns:**  
  Each player starts with a fixed number of cards and takes turns playing one card **counter-clockwise**.

- **Tricks:**  
  A **trick** consists of 4 cards one played by each player.  
  The player who wins the trick collects the cards and the score they carry, and leads the next trick.

- **Winning a Trick:**  
  The first card played in a trick sets the **trick suit**.  
  The winner is the player who plays the **highest ranked card of that suit**.  
  The winner of the **final trick** also earns an **extra 10 points**.

---

### What This Tool Does

Below, you can select the cards held by each player. When you click **Calculate**, the tool computes the best sequence of plays for all players to **maximize the team score**, using a minimax search algorithm with alpha-beta pruning.

---

### Example Tricks

- `10♥`, `A♦`, `10♦`, `A♥` → **Winner: A♥**
- `Q♣`, `K♣`, `J♣`, `A♣` → **Winner: A♣**
            
---
""")

NUM_OF_STARTING_CARDS = st.number_input("**Number of Starting Cards**", 2, 6, 6)

for i in range(4):
    st.markdown(f"<h4 style='text-align: center; margin-bottom: 0.5rem;'>Player {i + 1}</h4>", unsafe_allow_html=True)
    player_cols = st.columns([1] * NUM_OF_STARTING_CARDS)

    for j, col in enumerate(player_cols):
        with col:
            st.selectbox(f"Card #{j + 1}", OPTIONS, key=f"player_{i}_card_{j}", index=i * 8 + j)

    st.markdown("<br>", unsafe_allow_html=True)

SUIT_CONVERSION = {
    "♥": 0,
    "♠": 1,
    "♦" : 2,
    "♣" : 3
}

RANK_CONVERSION = {
    "7": 7,
    "8": 8,
    "9": 9,
    "10": 10,
    "J": 11,
    "Q": 12,
    "K": 13,
    "A": 1,
}

def convert_card(card):
    suit = card[-1]
    rank = card[:-1]
    return SUIT_CONVERSION[suit] * 13 + RANK_CONVERSION[rank]

player_cards = []

for i in range(4):
    current_player = []
    for j in range(NUM_OF_STARTING_CARDS):
        current_player.append(convert_card(st.session_state[f"player_{i}_card_{j}"]))
    player_cards.append(current_player)


player_cards = np.array(player_cards)

player_cards.sort()

start = time.perf_counter()

game_state = GameState(player_cards.copy(), NUM_OF_STARTING_CARDS)

best_score, best_path = game_state_minimax(game_state, 4 * NUM_OF_STARTING_CARDS, -float("inf"), float("inf"), True, NUM_OF_STARTING_CARDS)

end = time.perf_counter()

left, mid, right = st.columns([1, 2, 1])
with mid:
    calculate = st.button("Calculate", type="primary", use_container_width=True)
    st.markdown("<br>", unsafe_allow_html=True)

if calculate:
    st.markdown("---")
    m1, m2, m3 = st.columns(3)
    m1.metric("Best Score", f"{best_score}")
    m2.metric("Nodes Visited", f"{nodes_visited:,}")
    m3.metric("Time", f"{end - start:.3f} s")

    st.markdown("<h2 style='text-align: center;'>Best Path</h2>", unsafe_allow_html=True)

    st.markdown("""
    <style>
        .card {
            display: inline-block;
            padding: 8px 12px;
            border: 1px solid rgba(0,0,0,.12);
            border-radius: 10px;
            background: #ffffff;
            font-weight: 600;
            margin: 0 auto;
            min-width: 64px;
            text-align: center;
            width: 100%;
        }
                
        .red {
            color: #d11a2a;
        }

        .blk {
            color: #000000;
        }
                
        .trick-title {
            margin: 10px 0 6px;
            padding: 6px 10px;
            border-left: 4px solid var(--primary-color,#32916F);
            background: rgba(50,145,111,.06);
            border-radius: 6px;
            font-weight: 600;
        }
                
        .player-label {
            font-size: .85rem;
            opacity: .75;
            margin-bottom: 4px;
            text-align: center;
        }
    </style>
    """, unsafe_allow_html=True)

    def render_card_html(card: int) -> str:
        suit_idx = int(get_suit(card))
        rank = int(get_rank(card))
        suits = ["♥", "♠", "♦", "♣"]
        suit = suits[suit_idx]
        if rank == 1:
            rank_txt = "A"
        elif rank == 13:
            rank_txt = "K"
        elif rank == 12:
            rank_txt = "Q"
        elif rank == 11:
            rank_txt = "J"
        else:
            rank_txt = str(rank)
        cls = "red" if suit in ("♥", "♦") else "blk"
        return f"<span class='card'><span class='{cls}'>{rank_txt}{suit}</span></span>"

    num_tricks = len(best_path) // 4
    for t in range(num_tricks):
        st.markdown(f"<div class='trick-title'>Trick {t+1}</div>", unsafe_allow_html=True)
        cols = st.columns(4)
        for p in range(4):
            with cols[p]:
                st.markdown(f"<div class='player-label'>Card {p+1}</div>", unsafe_allow_html=True)
                card_html = render_card_html(int(best_path[t*4 + p]))
                st.markdown(card_html, unsafe_allow_html=True)
