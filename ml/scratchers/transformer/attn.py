from typing import Optional
import math

import torch
import torch.nn as nn

class Cache():
    def __init__(self):
        self.queries = None
        self.keys = None
        self.values = None

    def update(self, queries, keys, values):
        if self.queries is None:
            self.queries = queries
            self.keys = keys
            self.values = values
        else:
            self.queries = torch.cat((self.queries, queries), dim=-2)
            self.keys = torch.cat((self.keys, keys), dim=-2)
            self.values = torch.cat((self.values, values), dim=-2)

    def __len__(self):
        if self.queries is not None:
            return self.queries.size(-2)
        else:
            return 0


class Attention(nn.Module):
    def __init__(
        self, 
        d_k: int,
        dropout: float = 0.2,
        bias: bool = True
    ):
        super().__init__()
        self.d_k = d_k
        self.dropout = dropout
        self.bias = bias
        self.attn_dropout = nn.Dropout(dropout)
        self.resid_dropout = nn.Dropout(dropout)
        self.linear_proj = nn.Linear(d_k, d_k, bias=bias)
        self.queries = nn.Linear(d_k, d_k, bias=bias)
        self.keys = nn.Linear(d_k, d_k, bias=bias)
        self.values = nn.Linear(d_k, d_k, bias=bias)
        self.std = math.sqrt(d_k)


    def forward(
        self, 
        x: torch.Tensor, 
        mask: torch.Tensor,
        cache: Optional[Cache]
    ):
        has_cache = cache is not None
        cache_len = len(cache) if has_cache else 0        
        x = x[:, cache_len:, :]
        mask = mask[:, cache_len:, :]

        q = self.queries(x)
        k = self.keys(x)
        v = self.values(x)

        if has_cache:
            cache.update(q, k, v)
            k = cache.keys
            v = cache.values

        attn_scores = torch.matmul(q, k.transpose(-2, -1)) / self.std
        attn_scores = attn_scores.masked_fill(mask == 0, value=float('-inf'))
        attn_scores = torch.softmax(attn_scores, dim=-1)
        attn_scores = self.attn_dropout(attn_scores)

        context_sequence = torch.matmul(attn_scores, v) # B x S x F
        out = self.resid_dropout(self.linear_proj(context_sequence))

        return out