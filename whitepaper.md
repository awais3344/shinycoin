Proof-of-work
=============================

ShinyCoin uses a novel hashing algorithm called **ramhog**.  It is designed to be ASIC and GPU resistant.  

Ramhog
-----
The algorithm is based on scrypt.  The issue with scrypt is that it does not use enough RAM.  Since a GPU is a massive parallel processor GPUs can leverage their parallelism to run many instances of scrypt simultaneously, and so scrypt is much more cost efficiently solved by a GPU.

Ramhog solves this by requiring much larger amounts of RAM - 1.6 gigabytes (as opposed to 128 kb as required by scrypt) with the parameters that ShinyCoin uses - yet still being fast enough to compute feasibly.  How? Instead of using xorsalsa to sequentially generate values, ramhog uses xorshift4096* ( http://xorshift.di.unimi.it/ ).  Xorshift PRNGs are among the fastest high-quality PRNGs known to exist, and all that is needed is an algorithm which generates a sequence of numbers which each depend on the previous numbers in the sequence, not necessarily an algorithm with good cryptographic hashing properties which is what xorsalsa was designed for.

One weakness of scrypt is that one only needs one block of the scratchpad in order to generate the following blocks of the scratchpad.  A solving algorithm could save on RAM by only storing intermediate values of the pad and generating needed values on-the-fly.  This would be slower, but a GPU or an ASIC could potentially more than make up the difference by their speed and parallelism.  Ramhog improves on this by pseudorandomly XORing values in the scratchpad with values earlier generated in the scratchpad.  To generate the 10000th value, one might have to look up the 6000th value, and the 6000th value might further depend on the 1000th value.  This, combined this with the fact that ramhog’s scratchpad has millions of values and that the PRNG state’s size is 64 times larger than one element of the scratchpad, makes a cache-miss with the storing-intermediates strategy extremely costly - the further into the scratchpad the value, the costlier.

Once the 1.6 gigabytes of scratchpad are generated, the final values are used to seed the xorshift PRNG again, which is then used to generate a sequence which randomly selects values from all over the scratchpad.  Since xorshift is so fast, this can be done a large number of times (1024*1024 times for ShinyCoin).  Such a high number of iterations leads to a high number of potentially extremely slow cache misses if sufficient RAM isn’t available to store all 1.6 gigabytes, which renders any variant of the algorithm that doesn’t store all values completely infeasible.  Thus any algorithm to generate ShinyCoin hashes will need access to 1.6 gigabytes of RAM.  

GPU- and ASCI-resistant
-----
Since a large amount of RAM on an ASIC would make it too costly , an ASIC implementation is totally out of the question.  GPUs will work, but won’t fare much better than CPUs.  A top-of-the-line GPU with 6 GB of RAM can only efficiently run three simultaneous instances -  A cheaper CPU with sufficient RAM can do the same.  GPUs are slower than equivalently-priced CPUs at sequential operations, so running the same number of instances at a time, the CPUs will be more efficient.

As ShinyCoin will eventually use proof-of-stake for security, the proof-of-work phase will only be relevant for the first year or so.  In that year, the hardware available to run proof-of-work mining will likely not change too drastically.  Once the majority of blocks are proof-of-stake it will not matter much if a ‘crack’ to ramhog gives GPUs a significant cost advantage over a CPU miner.  

Distribution
-----
The proof-of-work reward is defined by the following smooth exponentially-decaying function:

    200 / (block_height/52560)^2

The reward is capped at 200, so blocks before the 52560th block do not have exorbitant rewards.  A major problem with many new coins, including peercoin is that block rewards are astronomically larger in the beginning.  As coins are intent on being distributed fairly,  

The effect is to divide the reward by the square of the number of quarter-year periods so that the proof-of-work reward quickly grows insignificant.  If only proof-of-work blocks are generated:

       Period  |  Factor  |  Block Reward
     ----------+----------+----------------
     1/4 year  |  1       |  200.000000
     1/2 year  |  4       |   50.000000
     1 year    |  16      |   12.500000
     2 years   |  64      |    3.125000
     4 years   |  256     |    0.781250

Inflation after the first year is very little, especially compared with other crypto-currencies:

     Year | Coins generated that year | Inflation from PoW
    ------+---------------------------+--------------------
       1  |       18,395,943          |  -
       2  |        1,314,004          | 7.13%
       3  |          438,000          | 2.22%
       4  |          219,000          | 1.09%
       5  |          131,400          | 0.65%
       6  |           87,600          | 0.43%
       7  |           62,571          | 0.30%
       8  |           46,928          | 0.23%

The block height used in the reward calculation is only in terms of how many proof-of-work blocks have been mined, not blocks in total.  So, if one out of every two blocks is proof-of-stake, the schedule will take twice as long.  To keep the initial distribution period longer, proof-of-stake is only activated after three months worth of proof-of-work blocks.  

After this period, the goal is to transition to securing the network only by proof-of-stake, which is why the mining rewards drop off fairly quickly.  To further encourage this, the proof-of-stake rewards are subtracted from the proof-of-work rewards.  For example, if a proof-of-stake block creates 4 shinys, and the next proof-of-work reward should be 100, it will instead be 96.  The reward is reduced up to a quarter of what it should be, and the proof-of-stake debt carries over.

In implementing the proof-of-stake sha256 algorithm, Satoshi solved two problems that were difficult to solve.  First was a fair, wide distribution of coin.  Had the coins been “pre mined” (or at the time that word didn’t yet exist), and sold the coins there would be little chance the distribution of coin would be so widespread.  When a pre mined NXT was offered fairly to the world — anyone contributing will get their appropriate share of NXT — only 70 wallets managed to donate, and this for a total equivalent cost of a few thousand Euro — for 100% of NXT.  It is safe to say that at Bitcoin’s launch if the distribution depended on people reaching into their pockets, there would be very few people who would be willing to spend a very small amount of money collectively.  

ShinyCoin has the fairest possible distribution scheme.  Anybody can use their own computer and do proof-of-work for the initial distribution phase without being overtaken by centralized GPU and ASIC mining operations, after which anybody who holds ShinyCoins will get interest on their coins for helping to secure the network.  This was after all the clear intent of Satoshi Nakomoto with the implementation of the sha256 proof-of-work algorithm — fair distribution of coin to those willing to dedicate their computing power.  I doubt the author of a revolutionary decentralized ecash system wanted expensive specialized hardware in the hands of a few contributing no value and who’s result is to inflate the bitcoin supply and dilute the value for all other bitcoin holders.  

Security
----

If not proof of work, how is the network secure?.  Before Sunny King implemented proof-of-stake with Peercoin, there wasn’t a real world example of an alternative to proof-of-work to secure the network, so until that point proof-of-work was the best solution.  If one intention of proof-of-work was fair and wide distribution of coin, until early 2013 the plan worked very well, but at about the beginning of 2013 the “wide distribution” part has not even been close to true.  It is true that Bitcoin’s network is secure due to the proof-of-work mining — but at tremendous cost, and can be pointed out many multiples of hashing power “more secure” than is conceivably necessary.  Since general purpose computers became unprofitable, it is logical to conclude that at that point the fairest way to distribute coin was to simply not mint new coins, and permit the market to function.  

This is what is going on in reality minus the distraction of mining.  If someone wants €100 €1.000 or even €10.000 worth of Bitcoin he does not purchase mining equipment, rather signs up with an exchange or finds a local holder of coin to buy.  Since GPU farms and ASICs kicked the hashing power of Bitcoin into the stratosphere (early 2013) Bitcoin users suffered a 25% dilution at the hands of specialized miners, or in fiat terms €1 billion.  Bitcoin is a DAC (decentralized autonomous company) and its users (stakeholders) should be very concerned with efficiency and cost reduction where possible.  An even more unfortunate part is that since ASICS and GPUs are subject to competitive markets, and markets tend to drive profit margin down, only small percentage of this €1 billion as profit to the miners.  The rest went into research and development of a useless piece of junk if not for bitcoin mining.  Imagine someone burglarizing a house and stealing €1000 worth of things for his personal profit of just €150.  This is the vampiricism of Bitcoin mining.  

---

Proof-of-stake
-----
At some point there was no other provable way to secure the network, but Sunny King’s proof-of-stake implementation has changed that.  In the future there will be a proof-of-something else, or an even better implementation of a decentralized way to secure the network, but as of the date of this writing proof-of-stake is the most secure and cost effective way of securing the network.  The most fair and wide distribution of coin will be achieved with people contributing their own general purpose computer for their share of coin.  It is unlikely an entity would buy general purpose computers for the sake of mining a new coin, and quite expensive to rent for the sake of speculative mining.

The proof-of-stake phase starts 3 months in to ensure fair distribution.  Because of the nature of the system, once the proof-of-stake kicks in, proof-of-work rapidly becomes more difficult.  If proof-of-stake started right away, the very earliest adopters would begin minting proof-of-stake blocks, pushing out anyone from mining new coins via proof-of-work.  This limit ensures the early adoption period is a reasonable period of time.

The minimum coin age is 1 week instead of 1 month so more people can stake more of the time.  The maximum coin age is 2 months instead of 3 months to give newer coins a greater chance of minting a proof-of-stake block.  Note that the full mint reward is awarded even if the coin age is greater than 2 months - it’s just that the coins don’t have a greater chance of winning than if they were only 2 months old.

Info-Transactions
=============================

An issue with Bitcoin’s transaction history is that it is neither anonymous nor completely transparent.  100% voluntary transparency is the easier of the two to solve.  At the time of this writing, on Bitcoin there is no way for an entity to identify itself on the blockchain.  There is little reason for not doing so.  A blogger with a wallet to accept donations would want his address as easily recognizable as possible.  For a blogger to receive a donation @letstalkbitcoin is much simpler than 1NaoegWVQ1XhMYVok3Y17rEziqS8C1wBmw, for a merchant conducting business @walmart would make purchases easier.  The userid is only one of infinite fields possible on the blockchain.  Any field a user desires to be displayed on a wallet can be added (e.g.  email address, phone number, website etc ..)

To the more difficult side of complete anonymity, I plan to update the code with either an implementation of darksend or zerocoin.  As I’ve no time to work on this, and this code has been sitting idle for nearly a year, I’ve decided to release it with without all the protocol changes I would have liked to make.  If given the time in the near future I hope to finish all the proposed changes.  

**Sunny Prince**



