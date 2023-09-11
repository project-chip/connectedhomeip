/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller;

public class ClusterIDMapping {
    public interface BaseCluster {
        long getID();
        String getAttributeName(long id) throws NoSuchFieldError;
        String getEventName(long id) throws NoSuchFieldError;
        String getCommandName(long id) throws NoSuchFieldError;
        long getAttributeID(String name) throws IllegalArgumentException;
        long getEventID(String name) throws IllegalArgumentException;
        long getCommandID(String name) throws IllegalArgumentException;
    }

    public static BaseCluster getCluster(long clusterId) {
        if (clusterId == First.ID) {
            return new First();
        }
        if (clusterId == Second.ID) {
            return new Second();
        }
        if (clusterId == Third.ID) {
            return new Third();
        }return null;
    }
    public static class First implements BaseCluster {
        public static final long ID = 1L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SomeInteger(1L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class Second implements BaseCluster {
        public static final long ID = 2L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Fabrics(0L),
            SomeBytes(123L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class Third implements BaseCluster {
        public static final long ID = 3L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SomeEnum(10L),
            Options(20L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }}
