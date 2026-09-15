class BufferExchange {
public:
    explicit BufferExchange(std::size_t bodyCount);

    // Thread physique : buffer à remplir pour le pas en cours.
    StateSnapshot& writeSlot() noexcept { return *write_; }

    // Thread physique : appelé une fois le pas terminé.
    void publish();

    // Thread rendu : tente de récupérer un nouvel état publié.
    // Retourne false si rien de neuf depuis le dernier appel.
    bool tryAdvance();

    // Thread rendu uniquement, après un tryAdvance() réussi.
    const StateSnapshot& prev()   const noexcept { return *prev_; }
    const StateSnapshot& target() const noexcept { return *target_; }

private:
    std::array<StateSnapshot, 4> storage_;
    StateSnapshot* write_;   // possédé exclusivement par le thread physique
    StateSnapshot* prev_;    // possédé exclusivement par le thread rendu
    StateSnapshot* target_;  // possédé exclusivement par le thread rendu
    StateSnapshot* ready_ = nullptr; // slot partagé, protégé par mutex_
    bool hasNewData_ = false;
    std::mutex mutex_;
};