// Copyright (c) 2014-2019 Zano Project
// Copyright (c) 2014-2018 The Louisdor Project
// Copyright (c) 2012-2013 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "wallet_chain_shortener.h"

#define WALLET_EVERYBLOCK_SIZE                                        10
#define WALLET_EVERY_10_BLOCKS_SIZE                                   144
#define WALLET_EVERY_100_BLOCKS_SIZE                                  144
#define WALLET_EVERY_1000_BLOCKS_SIZE                                 144



void wallet_chain_shortener::clear()
{
  m_local_bc_size = 1;
  m_last_10_blocks.clear();
  m_last_144_blocks_every_10.clear();
  m_last_144_blocks_every_100.clear();
  m_last_144_blocks_every_1000.clear();

}
//----------------------------------------------------------------------------------------------------
uint64_t wallet_chain_shortener::get_blockchain_current_size() const 
{
  return m_local_bc_size;
}
//----------------------------------------------------------------------------------------------------
uint64_t wallet_chain_shortener::get_top_block_height() const
{
  return m_local_bc_size - 1; 
}
//----------------------------------------------------------------------------------------------------
void wallet_chain_shortener::push_new_block_id(const crypto::hash& id, uint64_t height)
{
  //primary 10
  //self check
  if (!m_last_10_blocks.empty())
  {
    THROW_IF_FALSE_WALLET_INT_ERR_EX(get_blockchain_current_size() == height, "Inernal error: get_blockchain_current_height(){" << get_blockchain_current_size() << "} == height{" << height << "} is not equal");
  }

  m_last_10_blocks[height] = id;
  if (m_last_10_blocks.size() > WALLET_EVERYBLOCK_SIZE)
  {
    m_last_10_blocks.erase(m_last_10_blocks.begin());
  }

  //every 10-th
  if (height % 10 == 0)
  {
    //self check
    if (!m_last_144_blocks_every_10.empty())
    {
      THROW_IF_FALSE_WALLET_INT_ERR_EX((--m_last_144_blocks_every_10.end())->first + 10 == height, "Inernal error: (--m_last_144_blocks_every_10.end())->first + 10{" << (--m_last_144_blocks_every_10.end())->first + 10 << "} == height{" << height << "} is not equal");
    }
    m_last_144_blocks_every_10[height] = id;
  }
  //every 100-th
  if (height % 100 == 0)
  {
    //self check
    if (!m_last_144_blocks_every_100.empty())
    {
      THROW_IF_FALSE_WALLET_INT_ERR_EX((--m_last_144_blocks_every_100.end())->first + 100 == height, "Inernal error: (--m_last_144_blocks_every_100.end())->first + 100{" << (--m_last_144_blocks_every_100.end())->first + 100 << "} == height{" << height << "} is not equal");
    }
    m_last_144_blocks_every_100[height] = id;
  }
  //every 1000-th
  //every 100-th
  if (height % 1000 == 0)
  {
    //self check
    if (!m_last_144_blocks_every_1000.empty())
    {
      THROW_IF_FALSE_WALLET_INT_ERR_EX((--m_last_144_blocks_every_1000.end())->first + 1000 == height, "Inernal error: (--m_last_144_blocks_every_1000.end())->first + 1000{" << (--m_last_144_blocks_every_1000.end())->first + 1000 << "} == height{" << height << "} is not equal");
    }
    m_last_144_blocks_every_1000[height] = id;
  }
}
//----------------------------------------------------------------------------------------------------
void wallet_chain_shortener::get_short_chain_history(std::list<crypto::hash>& ids)const 
{
  ids.clear();
  uint64_t i = 0;
  uint64_t sz = get_blockchain_current_size();
  if (!sz)
    return;

  //first put last 10
  for (auto it = m_last_10_blocks.rbegin(); it != m_last_10_blocks.rend(); it++)
  {
    ids.push_back(it->second);
    i = it->first;
  }

  uint64_t current_back_offset = m_last_10_blocks.size();
  //self check
  THROW_IF_FALSE_WALLET_INT_ERR_EX(current_back_offset == sz - i, "Inernal error: current_back_offset{" << current_back_offset << "} == sz-i{" << sz << " - " << i << "} is not equal");

  uint64_t current_offset_distance = 10;
  current_back_offset += 10;
  while (current_back_offset < sz)
  {
    uint64_t get_item_around = sz - current_back_offset;
    std::pair<uint64_t, crypto::hash> item = AUTO_VAL_INIT(item);
    if (!lookup_item_around(get_item_around, item))
      break;

    //readjust item current_back_offset 
    current_back_offset = sz - item.first;

    ids.push_back(item.second);
    current_offset_distance *= 2;
    current_back_offset += current_offset_distance;
  }
  ids.push_back(m_genesis);
}
//----------------------------------------------------------------------------------------------------
bool wallet_chain_shortener::lookup_item_around(uint64_t i, std::pair<uint64_t, crypto::hash>& result)const 
{
  //in which container we are looking for?
  uint64_t devider = 0;
  std::map<uint64_t, crypto::hash>* pcontainer;
  if (m_last_144_blocks_every_10.size() && i < m_last_144_blocks_every_10.begin()->first)
  {
    devider = 10;
    pcontainer = &m_last_144_blocks_every_10;
  }
  else if (m_last_144_blocks_every_100.size() && i < m_last_144_blocks_every_100.begin()->first)
  {
    devider = 100;
    pcontainer = &m_last_144_blocks_every_100;
  }
  else if (m_last_144_blocks_every_1000.size() && i < m_last_144_blocks_every_1000.begin()->first)
  {
    devider = 1000;
    pcontainer = &m_last_144_blocks_every_1000;
  }
  else
    return false;

  //look in every 10'th
  i = i - i % devider;
  auto it = pcontainer->find(i);
  //self check
  THROW_IF_FALSE_WALLET_INT_ERR_EX(it != pcontainer->end(),
    "Inernal error: index " << i << " not found for devider " << devider
    << " pcontainer={" << pcontainer->begin()->first << ":" << (--pcontainer->end())->first << "}");
  result = *it;
  return true;
}
//----------------------------------------------------------------------------------------------------
void wallet_chain_shortener::check_if_block_matched(uint64_t i, const crypto::hash& id, bool& block_found, bool& block_matched, bool& full_reset_needed)const
{
  if (!m_last_10_blocks.empty() && i > m_last_10_blocks.begin()->first)
  {
    //must be in short sequence (m_last_10_blocks)
    //self check
    WLT_THROW_IF_FALSE_WALLET_INT_ERR_EX((--m_last_10_blocks.end())->first >= i,
      "Inernal error: index " << i << " is not located in expected range of m_last_10_blocks={"
      << m_last_10_blocks.begin()->first << ":" << (--m_last_10_blocks.end())->first << "}");

    auto it = m_last_10_blocks.find(i);
    WLT_THROW_IF_FALSE_WALLET_INT_ERR_EX(it != m_last_10_blocks.end(),
      "Inernal error: filde to find index " << i << " in m_last_10_blocks={"
      << m_last_10_blocks.begin()->first << ":" << (--m_last_10_blocks.end())->first << "}");

    block_found = true;
    if (id == it->second)
      block_matched = true;
    else
      block_matched = false;
  }
  else
  {
    //lazy lookup
    std::pair<uint64_t, crypto::hash> result = AUTO_VAL_INIT(result);
    bool r = lookup_item_around(i, result);
    if (!r)
    {
      WLT_LOG_L0("Wallet is getting fully resynced due to unmatched block " << id << " at " << i);
      block_matched = block_found = false;
      full_reset_needed = true;
      return;
    }
    else
    {
      if (result.first == i)
      {
        block_found = true;
        if (result.second == id)
        {
          block_matched = true;
        }
        else
        {
          block_matched = false;
        }
      }
      else
      {
        block_found = false;
        block_matched = false;
      }
    }
  }
}
//----------------------------------------------------------------------------------------------------
void clean_map_from_items_above(std::map<uint64_t, crypto::hash>& container, uint64_t height)
{
  while (container.size() && (--container.end())->first > height)
  {
    container.erase(--container.end());
  }
}
void wallet_chain_shortener::detach(uint64_t height)
{
  clean_map_from_items_above(m_last_10_blocks, height);
  clean_map_from_items_above(m_last_144_blocks_every_10, height);
  clean_map_from_items_above(m_last_144_blocks_every_100, height);
  clean_map_from_items_above(m_last_144_blocks_every_1000, height);
}