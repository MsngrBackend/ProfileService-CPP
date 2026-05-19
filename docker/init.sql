CREATE EXTENSION IF NOT EXISTS "pgcrypto";

CREATE TYPE visibility_type AS ENUM ('everyone', 'contacts', 'nobody');

CREATE TABLE IF NOT EXISTS profiles (
    user_id UUID PRIMARY KEY,
    first_name VARCHAR(100),
    last_name VARCHAR(100),
    username VARCHAR(64) UNIQUE,
    bio TEXT,
    avatar_url TEXT,
    last_seen_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS privacy_settings (
    user_id UUID PRIMARY KEY REFERENCES profiles(user_id) ON DELETE CASCADE,
    profile_visibility visibility_type NOT NULL DEFAULT 'everyone',
    last_seen_visibility visibility_type NOT NULL DEFAULT 'everyone',
    avatar_visibility visibility_type NOT NULL DEFAULT 'everyone'
);

CREATE TABLE IF NOT EXISTS contacts (
    owner_id UUID REFERENCES profiles(user_id) ON DELETE CASCADE,
    contact_id UUID REFERENCES profiles(user_id) ON DELETE CASCADE,
    alias VARCHAR(100),
    created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    PRIMARY KEY (owner_id, contact_id)
);

CREATE TABLE IF NOT EXISTS favorites (
    user_id UUID REFERENCES profiles(user_id) ON DELETE CASCADE,
    chat_id UUID NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    PRIMARY KEY (user_id, chat_id)
);

CREATE TABLE IF NOT EXISTS notification_settings (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID REFERENCES profiles(user_id) ON DELETE CASCADE,
    chat_id UUID,
    muted BOOLEAN NOT NULL DEFAULT false,
    muted_until TIMESTAMPTZ,
    UNIQUE (user_id, chat_id)
);

CREATE INDEX IF NOT EXISTS idx_profiles_username ON profiles(username);
CREATE INDEX IF NOT EXISTS idx_contacts_owner ON contacts(owner_id);
CREATE INDEX IF NOT EXISTS idx_contacts_contact ON contacts(contact_id);
CREATE INDEX IF NOT EXISTS idx_notif_user_chat ON notification_settings(user_id, chat_id);
CREATE INDEX IF NOT EXISTS idx_favorites_user ON favorites(user_id);
